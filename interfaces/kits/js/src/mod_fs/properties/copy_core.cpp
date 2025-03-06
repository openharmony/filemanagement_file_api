/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "copy_core.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <limits>
#include <memory>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "datashare_helper.h"
#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "trans_listener_core.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace AppFileService::ModuleFileUri;
namespace fs = std::filesystem;
const std::string FILE_PREFIX_NAME = "file://";
const std::string NETWORK_PARA = "?networkid=";
const string PROCEDURE_COPY_NAME = "FileFSCopy";
const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
const std::string MEDIA = "media";
const int SLEEP_TIME = 100000;
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;
constexpr int BUF_SIZE = 1024;
constexpr size_t MAX_SIZE = 1024 * 1024 * 4;
constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(300);
std::recursive_mutex CopyCore::mutex_;
std::map<FileInfosCore, std::shared_ptr<CallbackObjectCore>> CopyCore::jsCbMap_;

static string GetModeFromFlags(unsigned int flags)
{
    const string readMode = "r";
    const string writeMode = "w";
    const string appendMode = "a";
    const string truncMode = "t";
    string mode = readMode;
    mode += (((flags & O_RDWR) == O_RDWR) ? writeMode : "");
    mode = (((flags & O_WRONLY) == O_WRONLY) ? writeMode : mode);
    if (mode != readMode) {
        mode += ((flags & O_TRUNC) ? truncMode : "");
        mode += ((flags & O_APPEND) ? appendMode : "");
    }
    return mode;
}

static int OpenSrcFile(const string &srcPth, std::shared_ptr<FileInfosCore> infos, int32_t &srcFd)
{
    Uri uri(infos->srcUri);
    if (uri.GetAuthority() == MEDIA) {
        std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
        sptr<FileIoToken> remote = new (std::nothrow) IRemoteStub<FileIoToken>();
        if (!remote) {
            HILOGE("Failed to get remote object");
            return ENOMEM;
        }
        dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
        if (!dataShareHelper) {
            HILOGE("Failed to connect to datashare");
            return E_PERMISSION;
        }
        srcFd = dataShareHelper->OpenFile(uri, GetModeFromFlags(O_RDONLY));
        if (srcFd < 0) {
            HILOGE("Open media uri by data share fail. ret = %{public}d", srcFd);
            return EPERM;
        }
    } else {
        srcFd = open(srcPth.c_str(), O_RDONLY);
        if (srcFd < 0) {
            HILOGE("Error opening src file descriptor. errno = %{public}d", errno);
            return errno;
        }
    }
    return ERRNO_NOERR;
}

static int SendFileCore(std::unique_ptr<DistributedFS::FDGuard> srcFdg,
                        std::unique_ptr<DistributedFS::FDGuard> destFdg,
                        std::shared_ptr<FileInfosCore> infos)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> sendFileReq = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (sendFileReq == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int64_t offset = 0;
    struct stat srcStat{};
    if (fstat(srcFdg->GetFD(), &srcStat) < 0) {
        HILOGE("Failed to get stat of file by fd: %{public}d ,errno = %{public}d", srcFdg->GetFD(), errno);
        return errno;
    }
    int32_t ret = 0;
    int64_t size = static_cast<int64_t>(srcStat.st_size);
    while (size >= 0) {
        ret = uv_fs_sendfile(nullptr, sendFileReq.get(), destFdg->GetFD(), srcFdg->GetFD(),
            offset, MAX_SIZE, nullptr);
        if (ret < 0) {
            HILOGE("Failed to sendfile by errno : %{public}d", errno);
            return errno;
        }
        if (infos != nullptr && infos->taskSignal != nullptr) {
            if (infos->taskSignal->CheckCancelIfNeed(infos->srcPath)) {
                HILOGE("===Copy Task Canceled Success");
                return ECANCELED;
            }
        }
        offset += static_cast<int64_t>(ret);
        size -= static_cast<int64_t>(ret);
        if (ret == 0) {
            break;
        }
    }
    if (size != 0) {
        HILOGE("The execution of the sendfile task was terminated, remaining file size %{public}" PRIu64, size);
        return EIO;
    }
    return ERRNO_NOERR;
}

bool CopyCore::IsValidUri(const std::string &uri)
{
    return uri.find(FILE_PREFIX_NAME) == 0;
}

bool CopyCore::ValidOperand(std::string uriStr)
{
    if (IsValidUri(uriStr)) {
        return true;
    }
    return false;
}

bool CopyCore::IsRemoteUri(const std::string &uri)
{
    // NETWORK_PARA
    return uri.find(NETWORK_PARA) != uri.npos;
}

bool CopyCore::IsDirectory(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGE("stat failed, errno is %{public}d", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

bool CopyCore::IsFile(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("stat failed, errno is %{public}d, ", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

bool CopyCore::IsMediaUri(const std::string &uriPath)
{
    Uri uri(uriPath);
    string bundleName = uri.GetAuthority();
    return bundleName == MEDIA;
}

tuple<int, uint64_t> CopyCore::GetFileSize(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("Stat failed.");
        return { errno, 0 };
    }
    return { ERRNO_NOERR, buf.st_size };
}

int CopyCore::CheckOrCreatePath(const std::string &destPath)
{
    std::error_code errCode;
    if (!filesystem::exists(destPath, errCode) && errCode.value() == ERRNO_NOERR) {
        HILOGI("destPath not exist");
        auto file = open(destPath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (file < 0) {
            HILOGE("Error opening file descriptor. errno = %{public}d", errno);
            return errno;
        }
        close(file);
    } else if (errCode.value() != 0) {
        return errCode.value();
    }
    return ERRNO_NOERR;
}

int CopyCore::CopyFile(const string &src, const string &dest, std::shared_ptr<FileInfosCore> infos)
{
    HILOGD("src = %{public}s, dest = %{public}s", GetAnonyString(src).c_str(), GetAnonyString(dest).c_str());
    int32_t srcFd = -1;
    int32_t ret = OpenSrcFile(src, infos, srcFd);
    if (srcFd < 0) {
        return ret;
    }
    auto destFd = open(dest.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (destFd < 0) {
        HILOGE("Error opening dest file descriptor. errno = %{public}d", errno);
        close(srcFd);
        return errno;
    }
    auto srcFdg = CreateUniquePtr<DistributedFS::FDGuard>(srcFd, true);
    auto destFdg = CreateUniquePtr<DistributedFS::FDGuard>(destFd, true);
    if (srcFdg == nullptr || destFdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(srcFd);
        close(destFd);
        return ENOMEM;
    }
    return SendFileCore(move(srcFdg), move(destFdg), infos);
}

int CopyCore::MakeDir(const string &path)
{
    filesystem::path destDir(path);
    std::error_code errCode;
    if (!filesystem::create_directory(destDir, errCode)) {
        HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

int CopyCore::CopySubDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfosCore> infos)
{
    std::error_code errCode;
    if (!filesystem::exists(destPath, errCode) && errCode.value() == ERRNO_NOERR) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    } else if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    uint32_t watchEvents = IN_MODIFY;
    if (infos->notifyFd >= 0) {
        int newWd = inotify_add_watch(infos->notifyFd, destPath.c_str(), watchEvents);
        if (newWd < 0) {
            HILOGE("inotify_add_watch, newWd is unvaild, newWd = %{public}d", newWd);
            return errno;
        }
        {
            std::lock_guard<std::recursive_mutex> lock(CopyCore::mutex_);
            auto iter = CopyCore::jsCbMap_.find(*infos);
            auto receiveInfo = CreateSharedPtr<ReceiveInfo>();
            if (receiveInfo == nullptr) {
                HILOGE("Failed to request heap memory.");
                return ENOMEM;
            }
            receiveInfo->path = destPath;
            if (iter == CopyCore::jsCbMap_.end() || iter->second == nullptr) {
                HILOGE("Failed to find infos, srcPath = %{public}s, destPath = %{public}s",
                    GetAnonyString(infos->srcPath).c_str(), GetAnonyString(infos->destPath).c_str());
                return UNKNOWN_ERR;
            }
            iter->second->wds.push_back({ newWd, receiveInfo });
        }
    }
    return RecurCopyDir(srcPath, destPath, infos);
}

static int FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return DISMATCH;
    }
    return MATCH;
}

struct NameList {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

static void Deleter(struct NameList *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

std::string CopyCore::GetRealPath(const std::string& path)
{
    fs::path tempPath(path);
    fs::path realPath{};
    for (const auto& component : tempPath) {
        if (component == ".") {
            continue;
        } else if (component == "..") {
            realPath = realPath.parent_path();
        } else {
            realPath /= component;
        }
    }
    return realPath.string();
}

uint64_t CopyCore::GetDirSize(std::shared_ptr<FileInfosCore> infos, std::string path)
{
    unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;

    long int size = 0;
    for (int i = 0; i < num; i++) {
        string dest = path + '/' + string((pNameList->namelist[i])->d_name);
        if ((pNameList->namelist[i])->d_type == DT_LNK) {
            continue;
        }
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            size += static_cast<int64_t>(GetDirSize(infos, dest));
        } else {
            struct stat st {};
            if (stat(dest.c_str(), &st) == -1) {
                return size;
            }
            size += st.st_size;
        }
    }
    return size;
}

int CopyCore::RecurCopyDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfosCore> infos)
{
    unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;

    for (int i = 0; i < num; i++) {
        string src = srcPath + '/' + string((pNameList->namelist[i])->d_name);
        string dest = destPath + '/' + string((pNameList->namelist[i])->d_name);
        if ((pNameList->namelist[i])->d_type == DT_LNK) {
            continue;
        }
        int ret = ERRNO_NOERR;
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            ret = CopySubDir(src, dest, infos);
        } else {
            infos->filePaths.insert(dest);
            ret = CopyFile(src, dest, infos);
        }
        if (ret != ERRNO_NOERR) {
            return ret;
        }
    }
    return ERRNO_NOERR;
}

int CopyCore::CopyDirFunc(const string &src, const string &dest, std::shared_ptr<FileInfosCore> infos)
{
    HILOGD("CopyDirFunc in, src = %{public}s, dest = %{public}s", GetAnonyString(src).c_str(),
        GetAnonyString(dest).c_str());
    size_t found = dest.find(src);
    if (found != std::string::npos && found == 0) {
        return EINVAL;
    }
    fs::path srcPath = fs::u8path(src);
    std::string dirName;
    if (srcPath.has_parent_path()) {
        dirName = srcPath.parent_path().filename();
    }
    string destStr = dest + "/" + dirName;
    return CopySubDir(src, destStr, infos);
}

int CopyCore::ExecLocal(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    if (infos->isFile) {
        if (infos->srcPath == infos->destPath) {
            HILOGE("The src and dest is same");
            return EINVAL;
        }
        int ret = CheckOrCreatePath(infos->destPath);
        if (ret != ERRNO_NOERR) {
            HILOGE("check or create fail, error code is %{public}d", ret);
            return ret;
        }
    }
    if (!infos->hasListener) {
        return ExecCopy(infos);
    }
    auto ret = SubscribeLocalListener(infos, callback);
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to subscribe local listener, errno = %{public}d", ret);
        return ret;
    }
    StartNotify(infos, callback);
    return ExecCopy(infos);
}

int CopyCore::SubscribeLocalListener(std::shared_ptr<FileInfosCore> infos,
                                     std::shared_ptr<CallbackObjectCore> callback)
{
    infos->notifyFd = inotify_init();
    if (infos->notifyFd < 0) {
        HILOGE("Failed to init inotify, errno:%{public}d", errno);
        return errno;
    }
    infos->eventFd = eventfd(0, EFD_CLOEXEC);
    if (infos->eventFd < 0) {
        HILOGE("Failed to init eventFd, errno:%{public}d", errno);
        return errno;
    }
    callback->notifyFd = infos->notifyFd;
    callback->eventFd = infos->eventFd;
    int newWd = inotify_add_watch(infos->notifyFd, infos->destPath.c_str(), IN_MODIFY);
    if (newWd < 0) {
        auto errCode = errno;
        HILOGE("Failed to add watch, errno = %{public}d, notifyFd: %{public}d, destPath: %{public}s", errno,
               infos->notifyFd, infos->destPath.c_str());
        CloseNotifyFdLocked(infos, callback);
        return errCode;
    }
    auto receiveInfo = CreateSharedPtr<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        HILOGE("Failed to request heap memory.");
        inotify_rm_watch(infos->notifyFd, newWd);
        CloseNotifyFdLocked(infos, callback);
        return ENOMEM;
    }
    receiveInfo->path = infos->destPath;
    callback->wds.push_back({ newWd, receiveInfo });
    if (!infos->isFile) {
        callback->totalSize = GetDirSize(infos, infos->srcPath);
        return ERRNO_NOERR;
    }
    auto [err, fileSize] = GetFileSize(infos->srcPath);
    if (err == ERRNO_NOERR) {
        callback->totalSize = fileSize;
    }
    return err;
}

std::shared_ptr<CallbackObjectCore> CopyCore::RegisterListener(const std::shared_ptr<FileInfosCore> &infos)
{
    auto callback = CreateSharedPtr<CallbackObjectCore>(infos->listenerCb);
    if (callback == nullptr) {
        HILOGE("Failed to request heap memory.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter != jsCbMap_.end()) {
        HILOGE("CopyCore::RegisterListener, already registered.");
        return nullptr;
    }
    jsCbMap_.insert({ *infos, callback });
    return callback;
}

void CopyCore::UnregisterListener(std::shared_ptr<FileInfosCore> fileInfos)
{
    if (fileInfos == nullptr) {
        HILOGE("fileInfos is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*fileInfos);
    if (iter == jsCbMap_.end()) {
        HILOGI("It is not be registered.");
        return;
    }
    jsCbMap_.erase(*fileInfos);
}

void CopyCore::ReceiveComplete(UvEntryCore *entry)
{
    if (entry == nullptr) {
        HILOGE("entry pointer is nullptr.");
        return;
    }
    auto processedSize = entry->progressSize;
    if (processedSize < entry->callback->maxProgressSize) {
        HILOGE("enter ReceiveComplete2");
        return;
    }
    entry->callback->maxProgressSize = processedSize;

    entry->callback->listenerCb(processedSize, entry->totalSize);
}

UvEntryCore *CopyCore::GetUVEntry(std::shared_ptr<FileInfosCore> infos)
{
    UvEntryCore *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = jsCbMap_.find(*infos);
        if (iter == jsCbMap_.end()) {
            HILOGE("Failed to find callback");
            return nullptr;
        }
        auto callback = iter->second;
        entry = new (std::nothrow) UvEntryCore(iter->second, infos);
        if (entry == nullptr) {
            HILOGE("entry ptr is nullptr.");
            return nullptr;
        }
        entry->progressSize = callback->progressSize;
        entry->totalSize = callback->totalSize;
    }
    return entry;
}

void CopyCore::OnFileReceive(std::shared_ptr<FileInfosCore> infos)
{
    UvEntryCore *entry = GetUVEntry(infos);
    if (entry == nullptr) {
        HILOGE("failed to get uv entry");
        return;
    }
    ReceiveComplete(entry);
}

std::shared_ptr<ReceiveInfo> CopyCore::GetReceivedInfo(int wd, std::shared_ptr<CallbackObjectCore> callback)
{
    auto it = find_if(callback->wds.begin(), callback->wds.end(), [wd](const auto& item) {
        return item.first == wd;
    });
    if (it != callback->wds.end()) {
        return it->second;
    }
    return nullptr;
}

bool CopyCore::CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfosCore> infos)
{
    return infos->filePaths.count(filePath) != 0;
}

int CopyCore::UpdateProgressSize(const std::string &filePath,
                                 std::shared_ptr<ReceiveInfo> receivedInfo,
                                 std::shared_ptr<CallbackObjectCore> callback)
{
    auto [err, fileSize] = GetFileSize(filePath);
    if (err != ERRNO_NOERR) {
        HILOGE("GetFileSize failed, err: %{public}d.", err);
        return err;
    }
    auto size = fileSize;
    auto iter = receivedInfo->fileList.find(filePath);
    if (iter == receivedInfo->fileList.end()) {
        receivedInfo->fileList.insert({ filePath, size });
        callback->progressSize += size;
    } else { // file
        if (size > iter->second) {
            callback->progressSize += (size - iter->second);
            iter->second = size;
        }
    }
    return ERRNO_NOERR;
}

std::shared_ptr<CallbackObjectCore> CopyCore::GetRegisteredListener(std::shared_ptr<FileInfosCore> infos)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter == jsCbMap_.end()) {
        HILOGE("It is not registered.");
        return nullptr;
    }
    return iter->second;
}

void CopyCore::CloseNotifyFd(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    callback->closed = false;
    infos->eventFd = -1;
    infos->notifyFd = -1;
    {
        std::unique_lock<std::mutex> lock(callback->cvLock);
        callback->CloseFd();
        callback->cv.notify_one();
    }
}

void CopyCore::CloseNotifyFdLocked(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    {
        lock_guard<mutex> lock(callback->readMutex);
        callback->closed = true;
        if (callback->reading) {
            HILOGE("close while reading");
            return;
        }
    }
    CloseNotifyFd(infos, callback);
}

tuple<bool, int, bool> CopyCore::HandleProgress(
    inotify_event *event, std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    if (callback == nullptr) {
        return { true, EINVAL, false };
    }
    auto receivedInfo = GetReceivedInfo(event->wd, callback);
    if (receivedInfo == nullptr) {
        return { true, EINVAL, false };
    }
    std::string fileName = receivedInfo->path;
    if (!infos->isFile) { // files under subdir
        fileName += "/" + string(event->name);
        if (!CheckFileValid(fileName, infos)) {
            return { true, EINVAL, false };
        }
        auto err = UpdateProgressSize(fileName, receivedInfo, callback);
        if (err != ERRNO_NOERR) {
            return { false, err, false };
        }
    } else {
        auto [err, fileSize] = GetFileSize(fileName);
        if (err != ERRNO_NOERR) {
            return { false, err, false };
        }
        callback->progressSize = fileSize;
    }
    return { true, callback->errorCode, true };
}

void CopyCore::ReadNotifyEvent(std::shared_ptr<FileInfosCore> infos)
{
    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *event = nullptr;
    int len = 0;
    int64_t index = 0;
    auto callback = GetRegisteredListener(infos);
    while (((len = read(infos->notifyFd, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {}
    while (infos->run && index < len) {
        event = reinterpret_cast<inotify_event *>(buf + index);
        auto [needContinue, errCode, needSend] = HandleProgress(event, infos, callback);
        if (!needContinue) {
            infos->exceptionCode = errCode;
            return;
        }
        if (needContinue && !needSend) {
            index += static_cast<int64_t>(sizeof(struct inotify_event) + event->len);
            continue;
        }
        if (callback->progressSize == callback->totalSize) {
            infos->run = false;
            return;
        }
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime >= infos->notifyTime) {
            OnFileReceive(infos);
            infos->notifyTime = currentTime + NOTIFY_PROGRESS_DELAY;
        }
        index += static_cast<int64_t>(sizeof(struct inotify_event) + event->len);
    }
}

void CopyCore::ReadNotifyEventLocked(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    {
        std::lock_guard<std::mutex> lock(callback->readMutex);
        if (callback->closed) {
            HILOGE("read after close");
            return;
        }
        callback->reading = true;
    }
    ReadNotifyEvent(infos);
    {
        std::lock_guard<std::mutex> lock(callback->readMutex);
        callback->reading = false;
        if (callback->closed) {
            HILOGE("close after read");
            CloseNotifyFd(infos, callback);
            return;
        }
    }
}

void CopyCore::GetNotifyEvent(std::shared_ptr<FileInfosCore> infos)
{
    auto callback = GetRegisteredListener(infos);
    if (callback == nullptr) {
        infos->exceptionCode = EINVAL;
        return;
    }
    prctl(PR_SET_NAME, "NotifyThread");
    nfds_t nfds = 2;
    struct pollfd fds[2];
    fds[0].events = 0;
    fds[1].events = POLLIN;
    fds[0].fd = infos->eventFd;
    fds[1].fd = infos->notifyFd;
    while (infos->run && infos->exceptionCode == ERRNO_NOERR && infos->eventFd != -1 && infos->notifyFd != -1) {
        auto ret = poll(fds, nfds, -1);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                infos->run = false;
                return;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEventLocked(infos, callback);
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            infos->exceptionCode = errno;
            return;
        }
        {
            std::unique_lock<std::mutex> lock(callback->cvLock);
            callback->cv.wait_for(lock, std::chrono::microseconds(SLEEP_TIME), [callback]() -> bool {
                return callback->notifyFd == -1;
            });
        }
    }
}

tuple<int, std::shared_ptr<FileInfosCore>> CopyCore::CreateFileInfos(
    const std::string &srcUri, const std::string &destUri, std::optional<CopyOptions> options)
{
    auto infos = CreateSharedPtr<FileInfosCore>();
    if (infos == nullptr) {
        HILOGE("Failed to request heap memory.");
        return { ENOMEM, nullptr };
    }
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    FileUri srcFileUri(infos->srcUri);
    infos->srcPath = srcFileUri.GetRealPath();
    FileUri dstFileUri(infos->destUri);
    infos->destPath = dstFileUri.GetPath();
    infos->srcPath = GetRealPath(infos->srcPath);
    infos->destPath = GetRealPath(infos->destPath);
    infos->isFile = IsMediaUri(infos->srcUri) || IsFile(infos->srcPath);
    infos->notifyTime = std::chrono::steady_clock::now() + NOTIFY_PROGRESS_DELAY;
    if (options != std::nullopt) {
        if (options.value().listenerCb) {
            infos->hasListener = true;
            infos->listenerCb = options.value().listenerCb;
        }
        if (options.value().taskSignalEntityCore != nullptr) {
            infos->taskSignal = options.value().taskSignalEntityCore->taskSignal_;
        }
    }

    return { ERRNO_NOERR, infos };
}

void CopyCore::StartNotify(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    if (infos->hasListener && callback != nullptr) {
        callback->notifyHandler = std::thread([infos] {
            GetNotifyEvent(infos);
            });
    }
}

int CopyCore::ExecCopy(std::shared_ptr<FileInfosCore> infos)
{
    if (infos->isFile && IsFile(infos->destPath)) {
        // copyFile
        return CopyFile(infos->srcPath.c_str(), infos->destPath.c_str(), infos);
    }
    if (!infos->isFile && IsDirectory(infos->destPath)) {
        if (infos->srcPath.back() != '/') {
            infos->srcPath += '/';
        }
        if (infos->destPath.back() != '/') {
            infos->destPath += '/';
        }
        // copyDir
        return CopyDirFunc(infos->srcPath.c_str(), infos->destPath.c_str(), infos);
    }
    return EINVAL;
}

int CopyCore::ValidParam(const string& src, const string& dest,
                         std::optional<CopyOptions> options,
                         std::shared_ptr<FileInfosCore> &fileInfos)
{
    auto succSrc = ValidOperand(src);
    auto succDest = ValidOperand(dest);
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires uri/uri");
        return E_PARAMS;
    }
    auto [errCode, infos] = CreateFileInfos(src, dest, options);
    if (errCode != ERRNO_NOERR) {
        return errCode;
    }
    fileInfos = infos;
    return ERRNO_NOERR;
}

void CopyCore::WaitNotifyFinished(std::shared_ptr<CallbackObjectCore> callback)
{
    if (callback != nullptr) {
        if (callback->notifyHandler.joinable()) {
            callback->notifyHandler.join();
        }
    }
}

void CopyCore::CopyComplete(std::shared_ptr<FileInfosCore> infos, std::shared_ptr<CallbackObjectCore> callback)
{
    if (callback != nullptr && infos->hasListener) {
        callback->progressSize = callback->totalSize;
        OnFileReceive(infos);
    }
}

FsResult<void> CopyCore::DoCopy(const string& src, const string& dest, std::optional<CopyOptions> &options)
{
    std::shared_ptr<FileInfosCore> infos = nullptr;
    auto result = ValidParam(src, dest, options, infos);
    if (result != ERRNO_NOERR) {
        return FsResult<void>::Error(result);
    }

    auto callback = RegisterListener(infos);
    if (callback == nullptr) {
        return FsResult<void>::Error(EINVAL);
    }

    if (IsRemoteUri(infos->srcUri)) {
        if (infos->taskSignal != nullptr) {
            infos->taskSignal->MarkRemoteTask();
        }
        auto ret = TransListenerCore::CopyFileFromSoftBus(infos->srcUri, infos->destUri,
                                                          infos, std::move(callback));
        if (ret != ERRNO_NOERR) {
            return FsResult<void>::Error(ret);
        } else {
            return FsResult<void>::Success();
        }
    }
    result = CopyCore::ExecLocal(infos, callback);
    CloseNotifyFdLocked(infos, callback);
    infos->run = false;
    WaitNotifyFinished(callback);
    if (result != ERRNO_NOERR) {
        infos->exceptionCode = result;
        return FsResult<void>::Error(infos->exceptionCode);
    }
    CopyComplete(infos, callback);
    if (infos->exceptionCode != ERRNO_NOERR) {
        return FsResult<void>::Error(infos->exceptionCode);
    } else {
        return FsResult<void>::Success();
    }
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS