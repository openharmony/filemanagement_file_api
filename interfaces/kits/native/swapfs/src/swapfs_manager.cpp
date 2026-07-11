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
#include <cstdio>
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
    if (config_.swapRootPath.empty()) {
        return SWAPFS_E_INVAL;
    }
    if (config_.swapRootPath == DEFAULT_SWAPFS_ROOT_PATH) {
        if (mkdir(DEFAULT_SWAPFS_TEMP_PATH, DIR_MODE) != 0 && errno != EEXIST) {
            return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
        }
    }
    if (mkdir(config_.swapRootPath.c_str(), DIR_MODE) != 0) {
        if (errno != EEXIST) {
            return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
        }
        return ValidateSwapRoot(config_.swapRootPath);
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    OHOS::UniqueFd rootFd(open(config_.swapRootPath.c_str(), flags));
    if (rootFd < 0) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    int ret = CreateMarkerFileAt(rootFd, ROOT_MARKER);
    if (ret != SWAPFS_E_OK) {
        return ret;
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
    (void)SwapfsSessionCleaner::RemoveSessionTree(sessionPath_);
    sessionPath_.clear();
}

int SwapfsManager::Destroy()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) { return SWAPFS_E_OK; }
        shuttingDown_ = true;
        HILOGI("[Swapfs] Destroy initiated, rejecting new ops");
    }

    bool clean = WaitForActiveOps(DESTROY_WAIT_TIMEOUT_MS);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) { return SWAPFS_E_OK; }
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

int SwapfsManager::PrepareForSwapOut(const OH_SwapfsSwapOutRequest *request, uint64_t &newKey,
    std::string &tmpPath, std::string &swapPath, bool &useDirectIo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        return SWAPFS_E_SHUTTING_DOWN;
    }
    useDirectIo = config_.useDirectIo;
    if (useDirectIo && !IsDioAligned(request->buffer, request->bufferSize)) {
        return SWAPFS_E_DIO_ALIGN;
    }
    if (removeAllInProgress_) {
        return SWAPFS_E_BUSY;
    }
    ++activeOps_;
    newKey = nextKeyId_++;
    tmpPath = sessionPath_ + "/data/" + std::to_string(newKey) + ".tmp";
    swapPath = sessionPath_ + "/data/" + std::to_string(newKey) + ".swap";
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
        return SWAPFS_E_INVAL;
    }
    uint64_t newKey = 0;
    std::string tmpPath;
    std::string swapPath;
    bool useDirectIo = false;
    int prepRet = PrepareForSwapOut(request, newKey, tmpPath, swapPath, useDirectIo);
    if (prepRet != SWAPFS_E_OK) {
        return prepRet;
    }
    ActiveOperationGuard operation(*this);
    int reserveRet = control_->ReserveSwapOut(request->bufferSize);
    if (reserveRet != SWAPFS_E_OK) {
        return reserveRet;
    }

    SyncWriteEngine writer;
    int ret = writer.Write(tmpPath, request->buffer, request->bufferSize, useDirectIo);
    if (ret == SWAPFS_E_OK && rename(tmpPath.c_str(), swapPath.c_str()) != 0) {
        ret = MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    if (ret != SWAPFS_E_OK) {
        (void)unlink(tmpPath.c_str());
        control_->CancelReservedSwapOut(request->bufferSize);
        return ret;
    }

    SwapKeyEntry entry;
    entry.keyId = newKey;
    entry.path = swapPath;
    entry.dataSize = request->bufferSize;
    entry.createTime = NowMs();
    entry.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
    CommitSwapOutEntry(entry, keyId);
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

int SwapfsManager::ExecuteSwapInRead(const SwapKeyEntry &entry, void *buffer, size_t readIoSize,
    bool useDirectIo)
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
    HILOGD("[Swapfs] SwapIn success, keyId: %{public}" PRIu64, request->keyId);
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
    if (unlink(pathToRemove.c_str()) == 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = entries_.find(keyId);
        if (iter == entries_.end()) {
            HILOGW("[Swapfs] FinishSwapIn key removed before deferred cleanup");
            return;
        }
        control_->OnEntryRemoved(removedSize, removedSize);
        entries_.erase(iter);
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        HILOGW("[Swapfs] FinishSwapIn key removed after deferred unlink failed");
        return;
    }
    iter->second.status = OH_SWAPFS_KEY_STATUS_ACTIVE;
    HILOGW("[Swapfs] FinishSwapIn deferred delete unlink failed, reverting status to ACTIVE");
}

int SwapfsManager::QueryData(uint64_t keyId, OH_SwapfsDataInfo *info)
{
    if (keyId == 0 || info == nullptr) {
        return SWAPFS_E_INVAL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        return SWAPFS_E_SHUTTING_DOWN;
    }
    auto iter = entries_.find(keyId);
    if (iter == entries_.end()) {
        return SWAPFS_E_KEY_NOT_FOUND;
    }
    const SwapKeyEntry &entry = iter->second;
    if (entry.status != OH_SWAPFS_KEY_STATUS_ACTIVE) {
        return SWAPFS_E_KEY_STATE_INVALID;
    }
    info->keyId = entry.keyId;
    info->dataSize = entry.dataSize;
    info->occupiedSize = entry.dataSize;
    info->createTime = entry.createTime;
    info->status = entry.status;
    info->canSwapIn = true;
    return SWAPFS_E_OK;
}

int SwapfsManager::GetStats(OH_SwapfsStats *stats)
{
    if (stats == nullptr) {
        return SWAPFS_E_INVAL;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return SWAPFS_E_INVAL;
        }
        if (shuttingDown_) {
            return SWAPFS_E_SHUTTING_DOWN;
        }
        ++activeOps_;
    }
    ActiveOperationGuard operation(*this);
    control_->FillStats(*stats);
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
    if (removed) {
        HILOGD("[Swapfs] RemoveData success, keyId: %{public}" PRIu64, entry.keyId);
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
        return SWAPFS_E_OK;
    }
    ActiveOperationGuard operation(*this);
    int unlinkErr = 0;
    bool removed = unlink(entry.path.c_str()) == 0;
    if (!removed) {
        unlinkErr = errno;
        HILOGE("[Swapfs] RemoveData unlink failed, errno: %{public}d", unlinkErr);
    }
    FinalizeRemoveEntry(entry, removed);
    if (!removed) {
        return MapErrno(unlinkErr, SwapfsErrContext::KEY_OPERATION);
    }
    return SWAPFS_E_OK;
}

int SwapfsManager::CollectRemovableEntries(std::vector<SwapKeyEntry> &entriesToRemove)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return SWAPFS_E_INVAL;
    }
    if (shuttingDown_) {
        return SWAPFS_E_SHUTTING_DOWN;
    }
    if (activeOps_ > 0) {
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
        return SWAPFS_E_OK;
    }

    ActiveOperationGuard operation(*this);
    std::vector<bool> removedEntries;
    removedEntries.reserve(entriesToRemove.size());
    int firstError = SWAPFS_E_OK;
    for (const auto &entry : entriesToRemove) {
        bool removed = unlink(entry.path.c_str()) == 0;
        removedEntries.emplace_back(removed);
        if (!removed && firstError == SWAPFS_E_OK) {
            firstError = MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
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
    return firstError;
}
} // namespace OHOS::FileManagement::Swapfs
