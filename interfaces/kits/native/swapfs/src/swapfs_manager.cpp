/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swapfs_manager.h"

#include <chrono>
#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "swapfs_err_mapper.h"
#include "swapfs_io_engine.h"
#include "swapfs_session_cleaner.h"
#include "tokenid_kit.h"
#include "unique_fd.h"

namespace OHOS::FileManagement::Swapfs {
namespace {
constexpr mode_t DIR_MODE = S_IRWXU;
constexpr mode_t LOCK_FILE_MODE = S_IRUSR | S_IWUSR;
constexpr const char *SWAPFS_DIR_NAME = "swapfs";
constexpr const char *ROOT_MARKER = ".swapfs-root";
constexpr const char *SESSION_MARKER = ".swapfs-session";
constexpr uint32_t DESTROY_WAIT_TIMEOUT_MS = 200;
constexpr uint32_t DESTRUCTOR_WAIT_TIMEOUT_MS = 100;
constexpr mode_t NON_OWNER_PERMS = S_IRWXG | S_IRWXO;
constexpr int COMMON_E_PERMISSION_SYS = 202;
constexpr size_t MAX_PATH_LENGTH = PATH_MAX - 1;

bool IsSystemApp()
{
    uint64_t fullTokenId = OHOS::IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
}

int CreateMarkerFileAt(int dirFd, const char *name)
{
    int flags = O_CREAT | O_CLOEXEC | O_EXCL | O_NOFOLLOW | O_WRONLY;
    OHOS::UniqueFd markerFd(openat(dirFd, name, flags, LOCK_FILE_MODE));
    if (markerFd < 0) {
        return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    return SWAPFS_E_OK;
}

std::string BuildSwapRootPath(std::string basePath)
{
    while (basePath.size() > 1 && basePath.back() == '/') {
        basePath.pop_back();
    }
    if (basePath == "/") {
        return basePath + SWAPFS_DIR_NAME;
    }
    return basePath + "/" + SWAPFS_DIR_NAME;
}

bool IsPrivateOwned(const struct stat &st, bool directory)
{
    bool expectedType = directory ? S_ISDIR(st.st_mode) : S_ISREG(st.st_mode);
    return expectedType && st.st_uid == geteuid() && (st.st_mode & NON_OWNER_PERMS) == 0;
}

int ValidateSwapRoot(const std::string &path)
{
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    OHOS::UniqueFd rootFd(open(path.c_str(), flags));
    struct stat rootStat {};
    if (rootFd < 0 || fstat(rootFd, &rootStat) != 0 || !IsPrivateOwned(rootStat, true)) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    OHOS::UniqueFd markerFd(openat(rootFd, ROOT_MARKER, O_RDONLY | O_CLOEXEC | O_NOFOLLOW));
    struct stat markerStat {};
    if (markerFd < 0 || fstat(markerFd, &markerStat) != 0 ||
        !IsPrivateOwned(markerStat, false)) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    return SWAPFS_E_OK;
}

std::string MakeRandomId()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937_64 gen((static_cast<uint64_t>(rd()) << 32U) ^ static_cast<uint64_t>(now));
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << dist(gen) << dist(gen);
    return oss.str();
}

int64_t NowMs()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

std::string BuildDataRoot(const std::string &sessionPath)
{
    return sessionPath + "/data";
}

int RemoveSwapFile(const std::string &path, const char *operation, bool ignoreMissing)
{
    if (unlink(path.c_str()) == 0) {
        return SWAPFS_E_OK;
    }
    if (ignoreMissing && errno == ENOENT) {
        return SWAPFS_E_OK;
    }
    HILOGE("[Swapfs] %{public}s unlink failed, errno: %{public}d", operation, errno);
    return MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
}
} // namespace

SwapfsManager::SwapfsManager()
    : SwapfsManager(std::make_unique<ProxySwapControlProvider>())
{}

SwapfsManager::SwapfsManager(std::unique_ptr<SwapControlProvider> controlProvider)
{
    if (controlProvider == nullptr) {
        control_ = std::make_unique<ProxySwapControlProvider>();
        return;
    }
    control_ = std::move(controlProvider);
}

SwapfsManager::~SwapfsManager()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) { return; }
        shuttingDown_ = true;
        HILOGI("[Swapfs] destructor initiated, rejecting new ops");
    }

    bool clean = WaitForActiveOps(DESTRUCTOR_WAIT_TIMEOUT_MS);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) { return; }
        if (clean) {
            RemoveSessionDir();
            HILOGI("[Swapfs] destructor cleanup success");
        } else {
            HILOGW("[Swapfs] destructor wait timed out, leaving data for session cleaner");
        }
        CloseSessionLock();
        entries_.clear();
        control_->OnAllEntriesRemoved();
        initialized_ = false;
        shuttingDown_ = false;
    }
}

void SwapfsManager::ResolveConfig(const OH_SwapfsConfig *config, SwapfsConfigInner &inner)
{
    inner = SwapfsConfigInner {};
    std::string basePath = DEFAULT_SWAPFS_TEMP_PATH;
    if (config != nullptr) {
        if (config->swapRootPath != nullptr && config->swapRootPath[0] != '\0') {
            basePath = config->swapRootPath;
        }
        if (config->spaceLimitBytes > 0) {
            inner.spaceLimitBytes = config->spaceLimitBytes;
        }
        inner.useDirectIo = config->useDirectIo;
    }
    inner.swapRootPath = BuildSwapRootPath(std::move(basePath));
    inner.managerId = MakeRandomId();
}

int SwapfsManager::PrepareSwapRoot()
{
    if (config_.swapRootPath.empty() || config_.swapRootPath.front() != '/' ||
        config_.swapRootPath.size() > MAX_PATH_LENGTH) {
        return SWAPFS_E_INVAL;
    }
    if (config_.swapRootPath == DEFAULT_SWAPFS_ROOT_PATH) {
        if (mkdir(DEFAULT_SWAPFS_TEMP_PATH, DIR_MODE) != 0 && errno != EEXIST) {
            return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
        }
    }
    size_t slash = config_.swapRootPath.rfind('/');
    std::string name = config_.swapRootPath.substr(slash + 1);
    std::string parent = slash == 0 ? "/" : config_.swapRootPath.substr(0, slash);
    char resolved[PATH_MAX] = {};
    if (realpath(parent.c_str(), resolved) == nullptr) {
        return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    std::string canonicalParent(resolved);
    config_.swapRootPath = canonicalParent == "/" ? canonicalParent + name :
        canonicalParent + "/" + name;
    if (mkdir(config_.swapRootPath.c_str(), DIR_MODE) != 0) {
        if (errno != EEXIST) {
            return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
        }
    } else {
        int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
        OHOS::UniqueFd rootFd(open(config_.swapRootPath.c_str(), flags));
        int markerRet = rootFd < 0 ? SWAPFS_E_PATH_UNAVAILABLE :
            CreateMarkerFileAt(rootFd, ROOT_MARKER);
        if (markerRet != SWAPFS_E_OK) {
            rootFd = OHOS::UniqueFd();
            if (rmdir(config_.swapRootPath.c_str()) != 0) {
                HILOGE("[Swapfs] rollback newly created root failed, errno: %{public}d", errno);
            }
            return markerRet;
        }
    }
    return ValidateSwapRoot(config_.swapRootPath);
}

int CreateSessionLayoutAt(
    int rootFd, const std::string &name, OHOS::UniqueFd &sessionFd)
{
    if (mkdirat(rootFd, name.c_str(), DIR_MODE) != 0) {
        return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    sessionFd = OHOS::UniqueFd(openat(rootFd, name.c_str(), flags));
    int ret = sessionFd < 0 ?
        MapErrno(errno, SwapfsErrContext::PATH_OPERATION) :
        CreateMarkerFileAt(sessionFd, SESSION_MARKER);
    if (ret != SWAPFS_E_OK) {
        sessionFd = OHOS::UniqueFd();
        (void)SwapfsSessionCleaner::RemoveSessionAt(rootFd, name);
        return ret;
    }
    if (mkdirat(sessionFd, "data", DIR_MODE) != 0) {
        int err = errno;
        sessionFd = OHOS::UniqueFd();
        (void)SwapfsSessionCleaner::RemoveSessionAt(rootFd, name);
        return MapErrno(err, SwapfsErrContext::PATH_OPERATION);
    }
    return SWAPFS_E_OK;
}

int SwapfsManager::PrepareSession()
{
    SwapfsSessionCleaner cleaner(config_.swapRootPath);
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;
    int ret = cleaner.AcquireCleanupLock(rootFd, cleanupLock);
    if (ret != SWAPFS_E_OK) {
        return ret;
    }
    ret = cleaner.CleanupInactiveSessionsLocked(rootFd);
    if (ret != SWAPFS_E_OK) {
        return ret;
    }
    return CreateSessionLocked(rootFd);
}

int SwapfsManager::CreateSessionLocked(int rootFd)
{
    std::string creatingName = ".creating-" + config_.managerId + "-" + MakeRandomId();
    std::string finalName = "session-" + config_.managerId + "-" + MakeRandomId();
    OHOS::UniqueFd creatingFd;
    int ret = CreateSessionLayoutAt(rootFd, creatingName, creatingFd);
    if (ret != SWAPFS_E_OK) {
        return ret;
    }
    int lockFlags = O_CREAT | O_CLOEXEC | O_EXCL | O_NOFOLLOW | O_RDWR;
    sessionLockFd_ = openat(creatingFd, "session.lock", lockFlags, LOCK_FILE_MODE);
    if (sessionLockFd_ < 0) {
        int err = errno;
        creatingFd = OHOS::UniqueFd();
        (void)SwapfsSessionCleaner::RemoveSessionAt(rootFd, creatingName);
        return MapErrno(err, SwapfsErrContext::PATH_OPERATION);
    }
    bool lockFailed = flock(sessionLockFd_, LOCK_EX | LOCK_NB) != 0;
    if (lockFailed || renameat(rootFd, creatingName.c_str(), rootFd, finalName.c_str()) != 0) {
        int err = errno;
        CloseSessionLock();
        creatingFd = OHOS::UniqueFd();
        (void)SwapfsSessionCleaner::RemoveSessionAt(rootFd, creatingName);
        return MapErrno(err, SwapfsErrContext::PATH_OPERATION);
    }
    sessionPath_ = config_.swapRootPath + "/" + finalName;
    return SWAPFS_E_OK;
}

int SwapfsManager::Init(const OH_SwapfsConfig *config)
{
    if (!IsSystemApp()) {
        HILOGE("[Swapfs] Init rejected, caller is not a system app");
        return COMMON_E_PERMISSION_SYS;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        HILOGW("[Swapfs] already initialized, skip");
        return SWAPFS_E_BUSY;
    }
    ResolveConfig(config, config_);
    int ret = PrepareSwapRoot();
    if (ret != SWAPFS_E_OK) {
        HILOGE("[Swapfs] PrepareSwapRoot mkdir failed, ret: %{public}d", ret);
        return ret;
    }
    ret = PrepareSession();
    if (ret != SWAPFS_E_OK) {
        RemoveSessionDir();
        return ret;
    }

    ret = control_->Init(config_.spaceLimitBytes, DEFAULT_SPACE_CHECK_INTERVAL_MS);
    if (ret != SWAPFS_E_OK) {
        HILOGE("[Swapfs] control Init failed, ret: %{public}d", ret);
        RemoveSessionDir();
        CloseSessionLock();
        return ret;
    }
    initialized_ = true;
    HILOGI("[Swapfs] Init success");
    return SWAPFS_E_OK;
}

void SwapfsManager::CloseSessionLock()
{
    if (sessionLockFd_ >= 0) {
        (void)flock(sessionLockFd_, LOCK_UN);
        (void)close(sessionLockFd_);
        sessionLockFd_ = -1;
    }
}

void SwapfsManager::RemoveSessionDir()
{
    if (sessionPath_.empty()) {
        return;
    }
    int ret = SwapfsSessionCleaner::RemoveSessionTree(sessionPath_, config_.swapRootPath);
    if (ret != SWAPFS_E_OK) {
        HILOGE("[Swapfs] RemoveSessionDir failed, ret: %{public}d", ret);
    }
    sessionPath_.clear();
}

int SwapfsManager::Destroy()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shuttingDown_ = true;
        HILOGI("[Swapfs] Destroy initiated, rejecting new ops");
    }

    bool clean = WaitForActiveOps(DESTROY_WAIT_TIMEOUT_MS);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!clean) {
            shuttingDown_ = false;
            HILOGW("[Swapfs] Destroy E_BUSY, shuttingDown reset");
            return SWAPFS_E_BUSY;
        }
        RemoveSessionDir();
        CloseSessionLock();
        entries_.clear();
        control_->OnAllEntriesRemoved();
        initialized_ = false;
        shuttingDown_ = false;
        HILOGI("[Swapfs] Destroy success");
        return SWAPFS_E_OK;
    }
}

bool SwapfsManager::AllEntriesCleanLocked()
{
    return activeOps_ == 0;
}

bool SwapfsManager::WaitForActiveOps(uint32_t timeoutMs)
{
    auto timeout = std::chrono::milliseconds(timeoutMs);
    std::unique_lock<std::mutex> lock(mutex_);
    bool clean = activeOpsCv_.wait_for(lock, timeout, [this] {
        return AllEntriesCleanLocked();
    });
    if (clean) {
        HILOGI("[Swapfs] all ops completed, proceeding with cleanup");
        return true;
    }
    HILOGW("[Swapfs] wait timed out after %{public}u ms", timeoutMs);
    return false;
}

void SwapfsManager::EndOperation()
{
    bool notify = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        --activeOps_;
        notify = AllEntriesCleanLocked();
    }
    if (notify) {
        activeOpsCv_.notify_all();
    }
}

int SwapfsManager::PrepareForSwapOut(const OH_SwapfsSwapOutRequest *request,
    SwapOutContext &context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        HILOGW("[Swapfs] SwapOut not initialized");
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        HILOGW("[Swapfs] SwapOut rejected, manager shutting down");
        return SWAPFS_E_SHUTTING_DOWN;
    }
    context.useDirectIo = config_.useDirectIo;
    if (context.useDirectIo) {
        if (!IsDioAligned(request->buffer, request->bufferSize)) {
            HILOGW("[Swapfs] SwapOut DIO alignment check failed");
            return SWAPFS_E_DIO_ALIGN;
        }
    }
    if (removeAllInProgress_) {
        HILOGW("[Swapfs] SwapOut rejected, RemoveAllData in progress");
        return SWAPFS_E_BUSY;
    }
    ++activeOps_;
    context.keyId = nextKeyId_++;
    std::string dataRoot = BuildDataRoot(sessionPath_);
    context.tmpPath = dataRoot + "/" + std::to_string(context.keyId) + ".tmp";
    context.swapPath = dataRoot + "/" + std::to_string(context.keyId) + ".swap";
    return SWAPFS_E_OK;
}

void SwapfsManager::CommitSwapOutEntry(const SwapKeyEntry &entry, uint64_t *keyId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    entries_[entry.keyId] = entry;
    control_->OnSwapOutCommitted(entry.dataSize, entry.dataSize);
    *keyId = entry.keyId;
}

int SwapfsManager::SwapOut(const OH_SwapfsSwapOutRequest *request, uint64_t *keyId)
{
    if (request == nullptr || request->buffer == nullptr ||
        request->bufferSize == 0 || keyId == nullptr) {
        HILOGW("[Swapfs] SwapOut invalid params");
        return SWAPFS_E_INVAL;
    }
    SwapOutContext context;
    int prepRet = PrepareForSwapOut(request, context);
    if (prepRet != SWAPFS_E_OK) {
        return prepRet;
    }
    ActiveOperationGuard operation(*this);
    int reserveRet = control_->ReserveSwapOut(request->bufferSize);
    if (reserveRet != SWAPFS_E_OK) {
        HILOGW("[Swapfs] SwapOut reserve failed, ret: %{public}d", reserveRet);
        return reserveRet;
    }

    SyncWriteEngine writer;
    int ret = writer.Write(
        context.tmpPath, request->buffer, request->bufferSize, context.useDirectIo);
    if (ret != SWAPFS_E_OK) {
        HILOGE("[Swapfs] SwapOut write failed, ret: %{public}d", ret);
    }
    if (ret == SWAPFS_E_OK && rename(context.tmpPath.c_str(), context.swapPath.c_str()) != 0) {
        HILOGE("[Swapfs] SwapOut rename failed, errno: %{public}d", errno);
        ret = MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    if (ret != SWAPFS_E_OK) {
        (void)RemoveSwapFile(context.tmpPath, "SwapOut tmp", true);
        control_->CancelReservedSwapOut(request->bufferSize);
        return ret;
    }

    SwapKeyEntry entry;
    entry.keyId = context.keyId;
    entry.path = context.swapPath;
    entry.dataSize = request->bufferSize;
    entry.createTime = NowMs();
    entry.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
    CommitSwapOutEntry(entry, keyId);
    HILOGI("[Swapfs] SwapOut success, keyId: %{public}" PRIu64 ", size: %{public}" PRIu64,
        *keyId, request->bufferSize);
    return SWAPFS_E_OK;
}

int SwapfsManager::PrepareForSwapIn(bool &useDirectIo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        HILOGW("[Swapfs] SwapIn not initialized");
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        HILOGW("[Swapfs] SwapIn rejected, manager shutting down");
        return SWAPFS_E_SHUTTING_DOWN;
    }
    useDirectIo = config_.useDirectIo;
    ++activeOps_;
    return SWAPFS_E_OK;
}

int SwapfsManager::LookupKeyForSwapIn(uint64_t keyId, SwapKeyEntry &entry)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        HILOGW("[Swapfs] SwapIn key not found, keyId: %{public}" PRIu64, keyId);
        return SWAPFS_E_KEY_NOT_FOUND;
    }
    if (iter->second.status != OH_SWAPFS_KEY_STATUS_ACTIVE) {
        HILOGW("[Swapfs] SwapIn key state invalid");
        return SWAPFS_E_KEY_STATE_INVALID;
    }
    ++iter->second.readCount;
    entry = iter->second;
    return SWAPFS_E_OK;
}

int SwapfsManager::ExecuteSwapInRead(
    const SwapKeyEntry &entry, void *buffer, size_t readIoSize, bool useDirectIo)
{
    if (useDirectIo) {
        if (uringReader_.IsAvailable()) {
            HILOGD("[Swapfs] Use io_uring");
            return uringReader_.Read(entry.path, buffer, readIoSize, 0);
        }
        HILOGD("[Swapfs] Use SyncDio");
        SyncReadEngine reader;
        return reader.Read(entry.path, buffer, readIoSize, 0, true);
    }
    SyncReadEngine reader;
    return reader.Read(entry.path, buffer, readIoSize, 0, false);
}

int SwapfsManager::SwapIn(const OH_SwapfsSwapInRequest *request, uint64_t *readSize)
{
    if (request == nullptr || request->keyId == 0 ||
        request->buffer == nullptr || request->bufferSize == 0) {
        HILOGW("[Swapfs] SwapIn invalid params");
        return SWAPFS_E_INVAL;
    }

    bool useDirectIo = false;
    int prepRet = PrepareForSwapIn(useDirectIo);
    if (prepRet != SWAPFS_E_OK) {
        return prepRet;
    }
    ActiveOperationGuard operation(*this);
    if (useDirectIo && !IsDioAligned(request->buffer, request->bufferSize)) {
        HILOGW("[Swapfs] SwapIn DIO alignment check failed");
        return SWAPFS_E_DIO_ALIGN;
    }

    SwapKeyEntry entry;
    int lookupRet = LookupKeyForSwapIn(request->keyId, entry);
    if (lookupRet != SWAPFS_E_OK) {
        return lookupRet;
    }

    if (request->bufferSize < entry.dataSize) {
        HILOGW("[Swapfs] SwapIn buffer too small");
        FinishSwapIn(request->keyId);
        return SWAPFS_E_BUFFER_TOO_SMALL;
    }
    int ret = ExecuteSwapInRead(
        entry, request->buffer, static_cast<size_t>(entry.dataSize), useDirectIo);
    if (ret != SWAPFS_E_OK) {
        HILOGE("[Swapfs] SwapIn read failed, ret: %{public}d", ret);
        FinishSwapIn(request->keyId);
        return ret;
    }
    if (readSize != nullptr) {
        *readSize = entry.dataSize;
    }
    HILOGI("[Swapfs] SwapIn success, keyId: %{public}" PRIu64 ", size: %{public}" PRIu64,
        request->keyId, entry.dataSize);
    FinishSwapIn(request->keyId);
    return SWAPFS_E_OK;
}

void SwapfsManager::FinishSwapIn(uint64_t keyId)
{
    std::string pathToRemove;
    uint64_t removedSize = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = entries_.find(keyId);
        if (iter == entries_.end()) {
            HILOGW("[Swapfs] FinishSwapIn key not found");
            return;
        }
        --iter->second.readCount;
        if (iter->second.status != OH_SWAPFS_KEY_STATUS_REMOVING ||
            iter->second.readCount > 0) {
            return;
        }
        pathToRemove = iter->second.path;
        removedSize = iter->second.dataSize;
    }
    bool removed = RemoveSwapFile(pathToRemove, "FinishSwapIn", false) == SWAPFS_E_OK;
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        HILOGW("[Swapfs] FinishSwapIn key removed before deferred cleanup");
        return;
    }
    if (!removed) {
        iter->second.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
        HILOGW("[Swapfs] FinishSwapIn deferred delete failed, reverting status to ACTIVE");
        return;
    }
    control_->OnEntryRemoved(removedSize, removedSize);
    entries_.erase(iter);
}

int SwapfsManager::QueryData(uint64_t keyId, OH_SwapfsDataInfo *info)
{
    if (keyId == 0 || info == nullptr) {
        HILOGW("[Swapfs] QueryData invalid params");
        return SWAPFS_E_INVAL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        HILOGW("[Swapfs] QueryData not initialized");
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        HILOGW("[Swapfs] QueryData rejected, manager shutting down");
        return SWAPFS_E_SHUTTING_DOWN;
    }
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        HILOGW("[Swapfs] QueryData key not found, keyId: %{public}" PRIu64, keyId);
        return SWAPFS_E_KEY_NOT_FOUND;
    }
    const SwapKeyEntry &entry = iter->second;
    if (entry.status != OH_SWAPFS_KEY_STATUS_ACTIVE) {
        HILOGW("[Swapfs] QueryData key state invalid");
        return SWAPFS_E_KEY_STATE_INVALID;
    }
    info->keyId = entry.keyId;
    info->dataSize = entry.dataSize;
    info->occupiedSize = entry.dataSize;
    info->createTime = entry.createTime;
    info->status = entry.status;
    info->canSwapIn = true;
    HILOGI("[Swapfs] QueryData success, keyId: %{public}" PRIu64 ", size: %{public}" PRIu64,
        keyId, entry.dataSize);
    return SWAPFS_E_OK;
}

int SwapfsManager::GetStats(OH_SwapfsStats *stats)
{
    if (stats == nullptr) {
        HILOGW("[Swapfs] GetStats invalid params");
        return SWAPFS_E_INVAL;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            HILOGW("[Swapfs] GetStats not initialized");
            return SWAPFS_E_INVAL;
        }
        if (shuttingDown_) {
            HILOGW("[Swapfs] GetStats rejected, manager shutting down");
            return SWAPFS_E_SHUTTING_DOWN;
        }
        ++activeOps_;
    }
    ActiveOperationGuard operation(*this);
    control_->FillStats(*stats);
    HILOGI("[Swapfs] GetStats success");
    return SWAPFS_E_OK;
}

int SwapfsManager::PrepareRemoveEntry(uint64_t keyId, SwapKeyEntry &entry)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        HILOGW("[Swapfs] RemoveData not initialized");
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        HILOGW("[Swapfs] RemoveData rejected, manager shutting down");
        return SWAPFS_E_SHUTTING_DOWN;
    }
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        HILOGW("[Swapfs] RemoveData key not found");
        return SWAPFS_E_KEY_NOT_FOUND;
    }
    if (iter->second.status != OH_SWAPFS_KEY_STATUS_ACTIVE) {
        HILOGW("[Swapfs] RemoveData key state invalid");
        return SWAPFS_E_KEY_STATE_INVALID;
    }
    iter->second.status = OH_SWAPFS_KEY_STATUS_REMOVING;
    if (iter->second.readCount == 0) {
        entry = iter->second;
        ++activeOps_;
    }
    return SWAPFS_E_OK;
}

void SwapfsManager::FinalizeRemoveEntry(const SwapKeyEntry &entry, bool removed)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = entries_.find(entry.keyId);
        if (iter == entries_.end()) {
            HILOGW("[Swapfs] RemoveData key removed before finalize");
            return;
        }
        if (removed) {
            control_->OnEntryRemoved(entry.dataSize, entry.dataSize);
            entries_.erase(iter);
        } else {
            iter->second.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
        }
    }
}

int SwapfsManager::RemoveData(uint64_t keyId)
{
    if (keyId == 0) {
        HILOGW("[Swapfs] RemoveData invalid keyId");
        return SWAPFS_E_INVAL;
    }
    SwapKeyEntry entry;
    int prepRet = PrepareRemoveEntry(keyId, entry);
    if (prepRet != SWAPFS_E_OK) {
        return prepRet;
    }
    if (entry.path.empty()) {
        HILOGI("[Swapfs] RemoveData deferred, keyId: %{public}" PRIu64, keyId);
        return SWAPFS_E_OK;
    }
    ActiveOperationGuard operation(*this);
    int removeRet = RemoveSwapFile(entry.path, "RemoveData", false);
    bool removed = removeRet == SWAPFS_E_OK;
    FinalizeRemoveEntry(entry, removed);
    if (!removed) {
        return removeRet;
    }
    HILOGI("[Swapfs] RemoveData success, keyId: %{public}" PRIu64, keyId);
    return SWAPFS_E_OK;
}

int SwapfsManager::CollectRemovableEntries(std::vector<SwapKeyEntry> &entriesToRemove)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        HILOGW("[Swapfs] RemoveAllData not initialized");
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        HILOGW("[Swapfs] RemoveAllData rejected, manager shutting down");
        return SWAPFS_E_SHUTTING_DOWN;
    }
    if (activeOps_ > 0) {
        HILOGW("[Swapfs] RemoveAllData rejected, active ops: %{public}u", activeOps_);
        return SWAPFS_E_BUSY;
    }
    entriesToRemove.reserve(entries_.size());
    for (const auto &item : entries_) {
        entriesToRemove.emplace_back(item.second);
    }
    for (auto &item : entries_) {
        item.second.status = OH_SWAPFS_KEY_STATUS_REMOVING;
    }
    if (!entriesToRemove.empty()) {
        removeAllInProgress_ = true;
        ++activeOps_;
    }
    return SWAPFS_E_OK;
}

int SwapfsManager::RemoveAllData()
{
    std::vector<SwapKeyEntry> entriesToRemove;
    int prepRet = CollectRemovableEntries(entriesToRemove);
    if (prepRet != SWAPFS_E_OK) {
        return prepRet;
    }
    if (entriesToRemove.empty()) {
        HILOGI("[Swapfs] RemoveAllData success, count: 0");
        return SWAPFS_E_OK;
    }

    ActiveOperationGuard operation(*this);
    std::vector<bool> removedEntries;
    removedEntries.reserve(entriesToRemove.size());
    int firstError = SWAPFS_E_OK;
    for (const auto &entry : entriesToRemove) {
        int removeRet = RemoveSwapFile(entry.path, "RemoveAllData", false);
        bool removed = removeRet == SWAPFS_E_OK;
        removedEntries.emplace_back(removed);
        if (!removed && firstError == SWAPFS_E_OK) {
            firstError = removeRet;
        }
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t index = 0; index < entriesToRemove.size(); ++index) {
            const auto &entry = entriesToRemove[index];
            auto iter = entries_.find(entry.keyId);
            if (iter == entries_.end()) {
                continue;
            }
            if (removedEntries[index]) {
                control_->OnEntryRemoved(entry.dataSize, entry.dataSize);
                entries_.erase(iter);
            } else {
                iter->second.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
            }
        }
        removeAllInProgress_ = false;
    }
    if (firstError != SWAPFS_E_OK) {
        return firstError;
    }
    HILOGI("[Swapfs] RemoveAllData success, count: %{public}" PRIu64,
        static_cast<uint64_t>(entriesToRemove.size()));
    return SWAPFS_E_OK;
}
} // namespace OHOS::FileManagement::Swapfs
