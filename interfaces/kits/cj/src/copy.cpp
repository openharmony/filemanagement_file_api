/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "copy.h"
#include <fcntl.h>
#include <filesystem>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include "datashare_helper.h"
#include "file_uri.h"
#include "file_utils.h"
#include "iservice_registry.h"
#include "translistener.h"

namespace OHOS {
namespace CJSystemapi {
using namespace FileFs;
namespace fs = std::filesystem;
const std::string NETWORK_PARA = "?networkid=";
const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;
constexpr int BUF_SIZE = 1024;
constexpr size_t MAX_SIZE = 1024 * 1024 * 128;
constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(300);
std::recursive_mutex CopyImpl::mutex_;
std::map<FileInfos, std::shared_ptr<CjCallbackObject>> CopyImpl::cjCbMap_;

static int OpenSrcFile(const std::string &srcPth, std::shared_ptr<FileInfos> infos, int32_t &srcFd)
{
    Uri uri(infos->srcUri);
    if (uri.GetAuthority() == "media") {
        sptr<FileIoToken> remote = new (std::nothrow) IRemoteStub<FileIoToken>();
        if (!remote) {
            LOGE("Failed to get remote object");
            return ENOMEM;
        }
        auto dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
        if (!dataShareHelper) {
            LOGE("Failed to connect to datashare");
            return E_PERMISSION;
        }
        srcFd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(O_RDONLY));
        if (srcFd < 0) {
            LOGE("Open media uri by data share fail. ret = %{public}d", srcFd);
            return EPERM;
        }
    } else {
        srcFd = open(srcPth.c_str(), O_RDONLY);
        if (srcFd < 0) {
            LOGE("Error opening src file descriptor. errno = %{public}d", errno);
            return errno;
        }
    }
    return ERRNO_NOERR;
}

static int SendFileCore(std::unique_ptr<DistributedFS::FDGuard> srcFdg,
                        std::unique_ptr<DistributedFS::FDGuard> destFdg,
                        std::shared_ptr<FileInfos> infos)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> sendFileReq = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (sendFileReq == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int64_t offset = 0;
    struct stat srcStat{};
    if (fstat(srcFdg->GetFD(), &srcStat) < 0) {
        LOGE("Failed to get stat of file by fd: %{public}d ,errno = %{public}d", srcFdg->GetFD(), errno);
        return errno;
    }
    int32_t ret = 0;
    int64_t size = static_cast<int64_t>(srcStat.st_size);
    while (size >= 0) {
        ret = uv_fs_sendfile(nullptr, sendFileReq.get(), destFdg->GetFD(), srcFdg->GetFD(),
            offset, MAX_SIZE, nullptr);
        if (ret < 0) {
            LOGE("Failed to sendfile by errno : %{public}d", errno);
            return errno;
        }
        if (infos != nullptr && infos->taskSignal != nullptr) {
            if (infos->taskSignal->CheckCancelIfNeed(infos->srcPath)) {
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
        LOGE("The execution of the sendfile task was terminated, remaining file size %{public}" PRIu64, size);
        return EIO;
    }
    return ERRNO_NOERR;
}

static int FilterFunc(const struct dirent *filename)
{
    if (std::string_view(filename->d_name) == "." || std::string_view(filename->d_name) == "..") {
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
}

std::string CopyImpl::GetRealPath(const std::string& path)
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

bool CopyImpl::IsFile(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGI("stat failed, errno is %{public}d, ", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

std::tuple<int, uint64_t> CopyImpl::GetFileSize(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGI("Stat failed.");
        return { errno, 0 };
    }
    return { ERRNO_NOERR, buf.st_size };
}

bool CopyImpl::CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfos> infos)
{
    return infos->filePaths.count(filePath) != 0;
}

int CopyImpl::UpdateProgressSize(const std::string &filePath,
    std::shared_ptr<ReceiveInfo> receivedInfo, std::shared_ptr<CjCallbackObject> callback)
{
    auto [err, fileSize] = GetFileSize(filePath);
    if (err != ERRNO_NOERR) {
        LOGE("GetFileSize failed, err: %{public}d.", err);
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

void CopyImpl::CheckOrCreatePath(const std::string &destPath)
{
    if (!std::filesystem::exists(destPath)) {
        LOGI("destPath not exist, destPath = %{public}s", destPath.c_str());
        auto file = open(destPath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (file < 0) {
            LOGE("Error opening file descriptor. errno = %{public}d", errno);
        }
        close(file);
    }
}

int CopyImpl::MakeDir(const std::string &path)
{
    std::filesystem::path destDir(path);
    std::error_code errCode;
    if (!std::filesystem::create_directory(destDir, errCode)) {
        LOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

int CopyImpl::CopySubDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos)
{
    if (!std::filesystem::exists(destPath)) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            LOGE("Failed to mkdir");
            return res;
        }
    }
    uint32_t watchEvents = IN_MODIFY;
    if (infos->notifyFd >= 0) {
        int newWd = inotify_add_watch(infos->notifyFd, destPath.c_str(), watchEvents);
        if (newWd < 0) {
            LOGE("inotify_add_watch, newWd is unvaild, newWd = %{public}d", newWd);
            return errno;
        }
        {
            std::lock_guard<std::recursive_mutex> lock(CopyImpl::mutex_);
            auto iter = CopyImpl::cjCbMap_.find(*infos);
            auto receiveInfo = FileManagement::CreateSharedPtr<ReceiveInfo>();
            if (receiveInfo == nullptr) {
                LOGE("Failed to request heap memory.");
                return ENOMEM;
            }
            receiveInfo->path = destPath;
            if (iter == CopyImpl::cjCbMap_.end() || iter->second == nullptr) {
                LOGE("Failed to find infos, srcPath = %{public}s, destPath = %{public}s", infos->srcPath.c_str(),
                    infos->destPath.c_str());
                return UNKNOWN_ERR;
            }
            iter->second->wds.push_back({ newWd, receiveInfo });
        }
    }
    return RecurCopyDir(srcPath, destPath, infos);
}

int CopyImpl::RecurCopyDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos)
{
    std::unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (std::nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;

    for (int i = 0; i < num; i++) {
        std::string src = srcPath + '/' + std::string((pNameList->namelist[i])->d_name);
        std::string dest = destPath + '/' + std::string((pNameList->namelist[i])->d_name);
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

bool CopyImpl::IsDirectory(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGE("stat failed, errno is %{public}d, path is %{public}s", errno, path.c_str());
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

int CopyImpl::CopyDirFunc(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos)
{
    LOGI("CopyDirFunc in, src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
    size_t found = dest.find(src);
    if (found != std::string::npos && found == 0) {
        return EINVAL;
    }
    fs::path srcPath = fs::u8path(src);
    std::string dirName;
    if (srcPath.has_parent_path()) {
        dirName = srcPath.parent_path().filename();
    }
    std::string destStr = dest + "/" + dirName;
    return CopySubDir(src, destStr, infos);
}

uint64_t CopyImpl::GetDirSize(std::shared_ptr<FileInfos> infos, std::string path)
{
    std::unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (std::nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;

    long int size = 0;
    for (int i = 0; i < num; i++) {
        std::string dest = path + '/' + std::string((pNameList->namelist[i])->d_name);
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

std::shared_ptr<FileInfos> CopyImpl::InitCjFileInfo(
    const std::string& srcUri, const std::string& destUri, sptr<CopyInfo> info)
{
    auto infos = FileManagement::CreateSharedPtr<FileInfos>();
    if (infos == nullptr) {
        LOGE("Failed to request heap memory by create FileInfos struct.");
        return nullptr;
    }
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    infos->listenerId = info->listenerId;
    infos->copySignalId = info->signalId;
    AppFileService::ModuleFileUri::FileUri srcFileUri(srcUri);
    AppFileService::ModuleFileUri::FileUri destFileUri(destUri);
    infos->srcPath = srcFileUri.GetRealPath();
    infos->destPath = destFileUri.GetPath();
    infos->srcPath = GetRealPath(infos->srcPath);
    infos->destPath = GetRealPath(infos->destPath);
    infos->notifyTime = std::chrono::steady_clock::now() + NOTIFY_PROGRESS_DELAY;
    if (info->listenerId > 0) {
        infos->hasListener = true;
    }
    auto signal = FFI::FFIData::GetData<TaskSignalImpl>(infos->copySignalId);
    if (signal != nullptr && signal->signalEntity != nullptr) {
        infos->taskSignal = signal->signalEntity->taskSignal_;
    }
    return infos;
}

void CopyImpl::ReceiveComplete(CProgress data,
    std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback)
{
    if (callback == nullptr) {
        LOGE("callback pointer is nullptr.");
        return;
    }
    auto processedSize = data.processedSize;
    if (processedSize < callback->maxProgressSize) {
        return;
    }
    callback->maxProgressSize = processedSize;

    if (callback->callback == nullptr) {
        LOGI("Empty callback.");
        return;
    }
    callback->callback(data);
}

void CopyImpl::OnFileReceive(std::shared_ptr<FileInfos> infos)
{
    auto callback = GetRegisteredListener(infos);
    if (callback == nullptr) {
        LOGE("failed to get listener progress");
        return;
    }
    CProgress data = {.processedSize = callback->progressSize,
                      .totalSize = callback->totalSize};
    ReceiveComplete(data, infos, callback);
}

std::shared_ptr<ReceiveInfo> CopyImpl::GetReceivedInfo(int wd, std::shared_ptr<CjCallbackObject> callback)
{
    auto it = find_if(callback->wds.begin(), callback->wds.end(), [wd](const auto& item) {
        return item.first == wd;
    });
    if (it != callback->wds.end()) {
        return it->second;
    }
    return nullptr;
}

int CopyImpl::CopyFile(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos)
{
    LOGI("src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
    int32_t srcFd = -1;
    int32_t ret = OpenSrcFile(src, infos, srcFd);
    if (srcFd < 0) {
        return ret;
    }
    auto destFd = open(dest.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (destFd < 0) {
        LOGE("Error opening dest file descriptor. errno = %{public}d", errno);
        close(srcFd);
        return errno;
    }
    auto srcFdg = FileManagement::CreateUniquePtr<DistributedFS::FDGuard>(srcFd, true);
    auto destFdg = FileManagement::CreateUniquePtr<DistributedFS::FDGuard>(destFd, true);
    if (srcFdg == nullptr || destFdg == nullptr) {
        LOGE("Failed to request heap memory.");
        close(srcFd);
        close(destFd);
        return ENOMEM;
    }
    return SendFileCore(move(srcFdg), move(destFdg), infos);
}

std::shared_ptr<CjCallbackObject> CopyImpl::GetRegisteredListener(std::shared_ptr<FileInfos> infos)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = cjCbMap_.find(*infos);
    if (iter == cjCbMap_.end()) {
        LOGE("It is not registered.");
        return nullptr;
    }
    return iter->second;
}

std::tuple<bool, int, bool> CopyImpl::HandleProgress(
    inotify_event *event, std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback)
{
    if (callback == nullptr) {
        return { true, EINVAL, false };
    }
    auto receivedInfo = GetReceivedInfo(event->wd, callback);
    if (receivedInfo == nullptr) {
        return { true, EINVAL, false };
    }
    std::string fileName = receivedInfo->path;
    if (event->len > 0) { // files under subdir
        fileName += "/" + std::string(event->name);
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

void CopyImpl::ReadNotifyEvent(std::shared_ptr<FileInfos> infos)
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

void CopyImpl::GetNotifyEvent(std::shared_ptr<FileInfos> infos)
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
                ReadNotifyEvent(infos);
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            infos->exceptionCode = errno;
            return;
        }
    }
}

void CopyImpl::StartNotify(std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback)
{
    if (infos->hasListener && callback != nullptr) {
        callback->notifyHandler = std::thread([infos] {
            GetNotifyEvent(infos);
            });
    }
}

int CopyImpl::ExecCopy(std::shared_ptr<FileInfos> infos)
{
    if (IsFile(infos->srcPath) && IsFile(infos->destPath)) {
        // copyFile
        return CopyFile(infos->srcPath.c_str(), infos->destPath.c_str(), infos);
    }
    if (IsDirectory(infos->srcPath) && IsDirectory(infos->destPath)) {
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

std::shared_ptr<CjCallbackObject> CopyImpl::RegisterListener(std::shared_ptr<FileInfos>& infos)
{
    auto callback = FileManagement::CreateSharedPtr<CjCallbackObject>(infos->listenerId);
    if (callback == nullptr) {
        LOGE("Failed to request heap memory by create callback object.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = cjCbMap_.find(*infos);
    if (iter != cjCbMap_.end()) {
        LOGE("Regist copy listener, already registered.")
        return nullptr;
    }
    cjCbMap_.insert({*infos, callback});
    return callback;
}

void CopyImpl::UnregisterListener(std::shared_ptr<FileInfos> infos)
{
    if (infos == nullptr) {
        LOGE("infos is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = cjCbMap_.find(*infos);
    if (iter == cjCbMap_.end()) {
        LOGI("It is not be registered.");
        return;
    }
    cjCbMap_.erase(*infos);
}

bool CopyImpl::IsRemoteUri(const std::string& uri)
{
    return uri.find(NETWORK_PARA) != uri.npos;
}

int64_t CopyImpl::DoCopy(std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback)
{
    if (IsRemoteUri(infos->srcUri)) {
        if (infos->taskSignal != nullptr) {
            infos->taskSignal->MarkRemoteTask();
        }
        auto ret = TransListener::CopyFileFromSoftBus(
            infos->srcUri, infos->destUri, infos, std::move(callback));
        return ret;
    }
    auto result = ExecLocal(infos, callback);
    CloseNotifyFd(infos, callback);
    infos->run = false;
    WaitNotifyFinished(callback);
    if (result != ERRNO_NOERR) {
        infos->exceptionCode = result;
        return infos->exceptionCode;
    }
    CopyComplete(infos, callback);
    return infos->exceptionCode;
}

int64_t CopyImpl::ExecLocal(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback)
{
    if (IsFile(infos->srcPath)) {
        if (infos->srcPath == infos->destPath) {
            LOGE("The src and dest is same, path = %{public}s", infos->srcPath.c_str());
            return EINVAL;
        }
        CheckOrCreatePath(infos->destPath);
    }
    if (!infos->hasListener) {
        return ExecCopy(infos);
    }
    auto ret = SubscribeLocalListener(infos, callback);
    if (ret != ERRNO_NOERR) {
        LOGE("Failed to subscribe local listener, errno = %{public}" PRIu64, ret);
        return ret;
    }
    StartNotify(infos, callback);
    return ExecCopy(infos);
}

int64_t CopyImpl::SubscribeLocalListener(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback)
{
    infos->notifyFd = inotify_init();
    if (infos->notifyFd < 0) {
        LOGE("Failed to init inotify, errno:%{public}d", errno);
        return errno;
    }
    infos->eventFd = eventfd(0, EFD_CLOEXEC);
    if (infos->eventFd < 0) {
        LOGE("Failed to init eventFd, errno:%{public}d", errno);
        return errno;
    }
    callback->notifyFd = infos->notifyFd;
    callback->eventFd = infos->eventFd;
    int newWd = inotify_add_watch(infos->notifyFd, infos->destPath.c_str(), IN_MODIFY);
    if (newWd < 0) {
        LOGE("Failed to add watch, errno = %{public}d, notifyFd: %{public}d, destPath: %{public}s",
             errno, infos->notifyFd, infos->destPath.c_str());
        CloseNotifyFd(infos, callback);
        return errno;
    }
    auto receiveInfo = FileManagement::CreateSharedPtr<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        LOGE("Failed to request heap memory.");
        inotify_rm_watch(infos->notifyFd, newWd);
        CloseNotifyFd(infos, callback);
        return ENOMEM;
    }
    receiveInfo->path = infos->destPath;
    callback->wds.push_back({ newWd, receiveInfo });
    if (IsDirectory(infos->srcPath)) {
        callback->totalSize = GetDirSize(infos, infos->srcPath);
        return ERRNO_NOERR;
    }
    auto [err, fileSize] = GetFileSize(infos->srcPath);
    if (err == ERRNO_NOERR) {
        callback->totalSize = fileSize;
    }
    return err;
}

void CopyImpl::CloseNotifyFd(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback)
{
    callback->CloseFd();
    infos->eventFd = -1;
    infos->notifyFd = -1;
}

void CopyImpl::WaitNotifyFinished(std::shared_ptr<CjCallbackObject>& callback)
{
    if (callback != nullptr) {
        if (callback->notifyHandler.joinable()) {
            callback->notifyHandler.join();
        }
    }
}

void CopyImpl::CopyComplete(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback)
{
    if (callback != nullptr && infos->hasListener) {
        callback->progressSize = callback->totalSize;
        OnFileReceive(infos);
    }
}

void CopyImpl::Copy(const char* srcUri, const char* destUri, sptr<CopyInfo> info)
{
    if (srcUri == nullptr || destUri == nullptr) {
        LOGE("Invalid input.");
        return;
    }
    std::string src(srcUri);
    std::string dest(destUri);
    std::shared_ptr<FileInfos> infos = InitCjFileInfo(src, dest, info);
    if (infos == nullptr) {
        return;
    }
    auto callback = RegisterListener(infos);
    if (callback == nullptr) {
        return;
    }
    DoCopy(infos, callback);
    UnregisterListener(infos);
}

CopyInfo::~CopyInfo() {}
}
}