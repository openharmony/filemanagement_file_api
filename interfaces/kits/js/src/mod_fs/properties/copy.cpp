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

#include "copy.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
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

#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "trans_listener.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace AppFileService::ModuleFileUri;
namespace fs = std::filesystem;
const std::string FILE_PREFIX_NAME = "file://";
const std::string NETWORK_PARA = "?networkid=";
const string PROCEDURE_COPY_NAME = "FileFSCopy";
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;
constexpr int BUF_SIZE = 1024;
constexpr size_t MAX_SIZE = 0x7ffff000;
constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(1000);
std::recursive_mutex Copy::mutex_;
std::map<FileInfos, std::shared_ptr<JsCallbackObject>> Copy::jsCbMap_;

bool Copy::IsValidUri(const std::string &uri)
{
    return uri.find(FILE_PREFIX_NAME) == 0;
}

tuple<bool, std::string> Copy::ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg)
{
    auto [succ, uri, ignore] = pathOrFdFromJsArg.ToUTF8String();
    if (!succ) {
        HILOGE("parse uri failed.");
        return { false, "" };
    }
    std::string uriStr = std::string(uri.get());
    if (IsValidUri(uriStr)) {
        return { true, uriStr };
    }
    return { false, "" };
}

tuple<bool, NVal> Copy::GetListenerFromOptionArg(napi_env env, const NFuncArg &funcArg)
{
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        NVal op(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("progressListener") && !op.GetProp("progressListener").TypeIs(napi_undefined)) {
            if (!op.GetProp("progressListener").TypeIs(napi_function)) {
                HILOGE("Illegal options.progressListener type");
                return { false, NVal() };
            }
            return { true, op.GetProp("progressListener") };
        }
    }
    return { true, NVal() };
}

tuple<bool, NVal> Copy::GetCopySignalFromOptionArg(napi_env env, const NFuncArg &funcArg)
{
    if (funcArg.GetArgc() < NARG_CNT::THREE) {
        return { true, NVal() };
    }
    NVal op(env, funcArg[NARG_POS::THIRD]);
    if (op.HasProp("copySignal") && !op.GetProp("copySignal").TypeIs(napi_undefined)) {
        if (!op.GetProp("copySignal").TypeIs(napi_object)) {
            HILOGE("Illegal options.CopySignal type");
            return { false, NVal() };
        }
        return { true, op.GetProp("copySignal") };
    }
    return { true, NVal() };
}

bool Copy::IsRemoteUri(const std::string &uri)
{
    // NETWORK_PARA
    return uri.find(NETWORK_PARA) != uri.npos;
}

bool Copy::IsDirectory(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGE("stat failed, errno is %{public}d, path is %{public}s", errno, path.c_str());
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

bool Copy::IsFile(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("stat failed, errno is %{public}d, ", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

tuple<int, uint64_t> Copy::GetFileSize(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("Stat failed.");
        return { errno, 0 };
    }
    return { ERRNO_NOERR, buf.st_size };
}

void Copy::CheckOrCreatePath(const std::string &destPath)
{
    if (!filesystem::exists(destPath)) {
        HILOGI("destPath not exist, destPath = %{public}s", destPath.c_str());
        auto file = open(destPath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (file < 0) {
            HILOGE("Error opening file descriptor. errno = %{public}d", errno);
        }
        close(file);
    }
}

int Copy::CopyFile(const string &src, const string &dest, std::shared_ptr<FileInfos> infos)
{
    HILOGD("src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
    auto srcFd = open(src.c_str(), O_RDONLY);
    auto destFd = open(dest.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (srcFd < 0 || destFd < 0) {
        HILOGE("Error opening file descriptor. errno = %{public}d", errno);
        close(srcFd);
        close(destFd);
        return errno;
    }
    auto srcFdg = CreateUniquePtr<DistributedFS::FDGuard>(srcFd, true);
    auto destFdg = CreateUniquePtr<DistributedFS::FDGuard>(destFd, true);
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> sendFileReq = {
        new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
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
    int64_t size = static_cast<int64_t>(srcStat.st_size);
    int ret = 0;
    while (size > 0) {
        ret = uv_fs_sendfile(nullptr, sendFileReq.get(), destFdg->GetFD(), srcFdg->GetFD(),
            offset, MAX_SIZE, nullptr);
        if (ret < 0) {
            HILOGE("Failed to sendfile by errno : %{public}d", errno);
            return errno;
        }
        if (infos != nullptr && infos->taskSignal != nullptr) {
            if (infos->taskSignal->CheckCancelIfNeed(src)) {
                return E_CANCELED;
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
        return E_IO;
    }
    return ERRNO_NOERR;
}

int Copy::MakeDir(const string &path)
{
    filesystem::path destDir(path);
    std::error_code errCode;
    if (!filesystem::create_directory(destDir, errCode)) {
        HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

int Copy::CopySubDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfos> infos)
{
    if (!filesystem::exists(destPath)) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    }
    uint32_t watchEvents = IN_MODIFY;
    if (infos->notifyFd >= 0) {
        int newWd = inotify_add_watch(infos->notifyFd, destPath.c_str(), watchEvents);
        if (newWd < 0) {
            HILOGE("inotify_add_watch, newWd is unvaild, newWd = %{public}d", newWd);
            return errno;
        }
        {
            std::lock_guard<std::recursive_mutex> lock(Copy::mutex_);
            auto iter = Copy::jsCbMap_.find(*infos);
            auto receiveInfo = CreateSharedPtr<ReceiveInfo>();
            if (receiveInfo == nullptr) {
                HILOGE("Failed to request heap memory.");
                return ENOMEM;
            }
            receiveInfo->path = destPath;
            if (iter == Copy::jsCbMap_.end() || iter->second == nullptr) {
                HILOGE("Failed to find infos, srcPath = %{public}s, destPath = %{public}s", infos->srcPath.c_str(),
                    infos->destPath.c_str());
                return UNKROWN_ERR;
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
}

std::string Copy::GetRealPath(const std::string& path)
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

uint64_t Copy::GetDirSize(std::shared_ptr<FileInfos> infos, const std::string &path)
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
            size += GetDirSize(infos, dest);
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

int Copy::RecurCopyDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfos> infos)
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

int Copy::CopyDirFunc(const string &src, const string &dest, std::shared_ptr<FileInfos> infos)
{
    HILOGD("CopyDirFunc in, src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
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

int Copy::ExecLocal(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    if (IsFile(infos->srcPath)) {
        if (infos->srcPath == infos->destPath) {
            HILOGE("The src and dest is same, path = %{public}s", infos->srcPath.c_str());
            return EINVAL;
        }
        CheckOrCreatePath(infos->destPath);
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

int Copy::SubscribeLocalListener(std::shared_ptr<FileInfos> infos,
                                 std::shared_ptr<JsCallbackObject> callback)
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
        CloseNotifyFd(infos, callback);
        return errCode;
    }
    auto receiveInfo = CreateSharedPtr<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        HILOGE("Failed to request heap memory.");
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

std::shared_ptr<JsCallbackObject> Copy::RegisterListener(napi_env env, const std::shared_ptr<FileInfos> &infos)
{
    auto callback = CreateSharedPtr<JsCallbackObject>(env, infos->listener);
    if (callback == nullptr) {
        HILOGE("Failed to request heap memory.");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter != jsCbMap_.end()) {
        HILOGE("Copy::RegisterListener, already registered.");
        return nullptr;
    }
    jsCbMap_.insert({ *infos, callback });
    return callback;
}

void Copy::UnregisterListener(std::shared_ptr<FileInfos> infos)
{
    if (infos == nullptr) {
        HILOGE("infos is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter == jsCbMap_.end()) {
        HILOGI("It is not be registered.");
        return;
    }
    jsCbMap_.erase(*infos);
}

void Copy::ReceiveComplete(uv_work_t *work, int stat)
{
    if (work == nullptr) {
        HILOGE("uv_work_t pointer is nullptr.");
        return;
    }

    std::shared_ptr<UvEntry> entry(static_cast<UvEntry *>(work->data), [work](UvEntry *data) {
        delete data;
        delete work;
    });
    if (entry == nullptr) {
        HILOGE("entry pointer is nullptr.");
        return;
    }
    auto processedSize = entry->progressSize;
    if (processedSize < entry->callback->maxProgressSize) {
        return;
    }
    entry->callback->maxProgressSize = processedSize;

    napi_handle_scope scope = nullptr;
    napi_env env = entry->callback->env;
    napi_status status = napi_open_handle_scope(env, &scope);
    if (status != napi_ok) {
        HILOGE("Failed to open handle scope, status: %{public}d.", status);
        return;
    }
    NVal obj = NVal::CreateObject(env);
    if (processedSize <= numeric_limits<int64_t>::max() && entry->totalSize <= numeric_limits<int64_t>::max()) {
        obj.AddProp("processedSize", NVal::CreateInt64(env, processedSize).val_);
        obj.AddProp("totalSize", NVal::CreateInt64(env, entry->totalSize).val_);
    }
    napi_value result = nullptr;
    napi_value jsCallback = entry->callback->nRef.Deref(env).val_;
    status = napi_call_function(env, nullptr, jsCallback, 1, &(obj.val_), &result);
    if (status != napi_ok) {
        HILOGE("Failed to get result, status: %{public}d.", status);
    }
    status = napi_close_handle_scope(env, scope);
    if (status != napi_ok) {
        HILOGE("Failed to close scope, status: %{public}d.", status);
    }
}

uv_work_t *Copy::GetUVwork(std::shared_ptr<FileInfos> infos)
{
    UvEntry *entry = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = jsCbMap_.find(*infos);
        if (iter == jsCbMap_.end()) {
            HILOGE("Failed to find callback");
            return nullptr;
        }
        auto callback = iter->second;
        infos->env = callback->env;
        entry = new (std::nothrow) UvEntry(iter->second, infos);
        if (entry == nullptr) {
            HILOGE("entry ptr is nullptr.");
            return nullptr;
        }
        entry->progressSize = callback->progressSize;
        entry->totalSize = callback->totalSize;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("Failed to create uv_work_t pointer");
        delete entry;
        return nullptr;
    }
    work->data = entry;
    return work;
}

void Copy::OnFileReceive(std::shared_ptr<FileInfos> infos)
{
    uv_work_t *work = GetUVwork(infos);
    if (work == nullptr) {
        HILOGE("failed to get uv work");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(infos->env, &loop);
    auto ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(ReceiveComplete));
    if (ret != 0) {
        HILOGE("failed to uv_queue_work");
        delete (reinterpret_cast<UvEntry *>(work->data));
        delete work;
    }
}

std::shared_ptr<ReceiveInfo> Copy::GetReceivedInfo(int wd, std::shared_ptr<JsCallbackObject> callback)
{
    auto it = find_if(callback->wds.begin(), callback->wds.end(), [wd](const auto& item) {
        return item.first == wd;
    });
    if (it != callback->wds.end()) {
        return it->second;
    }
    return nullptr;
}

bool Copy::CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfos> infos)
{
    return infos->filePaths.count(filePath) != 0;
}

int Copy::UpdateProgressSize(const std::string &filePath,
                             std::shared_ptr<ReceiveInfo> receivedInfo,
                             std::shared_ptr<JsCallbackObject> callback)
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

std::shared_ptr<JsCallbackObject> Copy::GetRegisteredListener(std::shared_ptr<FileInfos> infos)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter == jsCbMap_.end()) {
        HILOGE("It is not registered.");
        return nullptr;
    }
    return iter->second;
}

void Copy::CloseNotifyFd(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    callback->CloseFd();
    infos->eventFd = -1;
    infos->notifyFd = -1;
}

tuple<bool, int, bool> Copy::HandleProgress(
    inotify_event *event, std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    auto receivedInfo = GetReceivedInfo(event->wd, callback);
    if (receivedInfo == nullptr) {
        return { true, EINVAL, false };
    }
    std::string fileName = receivedInfo->path;
    if (event->len > 0) { // files under subdir
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
    return { true, ERRNO_NOERR, true };
}

void Copy::ReadNotifyEvent(std::shared_ptr<FileInfos> infos)
{
    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *event = nullptr;
    int len = 0;
    int index = 0;
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
            index += sizeof(struct inotify_event) + event->len;
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
        index += sizeof(struct inotify_event) + event->len;
    }
}

void Copy::GetNotifyEvent(std::shared_ptr<FileInfos> infos)
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
            if (fds[0].revents & POLLNVAL) {
                infos->run = false;
                return;
            }
            if (fds[1].revents & POLLIN) {
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

std::string Copy::ConvertUriToPath(const std::string &uri)
{
    FileUri fileUri(uri);
    return fileUri.GetRealPath();
}

tuple<int, std::shared_ptr<FileInfos>> Copy::CreateFileInfos(
    const std::string &srcUri, const std::string &destUri, const NVal &listener, NVal copySignal)
{
    auto infos = CreateSharedPtr<FileInfos>();
    if (infos == nullptr) {
        HILOGE("Failed to request heap memory.");
        return { ENOMEM, nullptr };
    }
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    infos->listener = listener;
    infos->env = listener.env_;
    infos->copySignal = copySignal;
    infos->srcPath = ConvertUriToPath(infos->srcUri);
    infos->destPath = ConvertUriToPath(infos->destUri);
    infos->srcPath = GetRealPath(infos->srcPath);
    infos->destPath = GetRealPath(infos->destPath);
    infos->notifyTime = std::chrono::steady_clock::now() + NOTIFY_PROGRESS_DELAY;
    if (listener) {
        infos->hasListener = true;
    }
    if (infos->copySignal) {
        auto taskSignalEntity = NClass::GetEntityOf<TaskSignalEntity>(infos->env, infos->copySignal.val_);
        if (taskSignalEntity != nullptr) {
            infos->taskSignal = taskSignalEntity->taskSignal_;
        }
    }
    return { ERRNO_NOERR, infos };
}

void Copy::StartNotify(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    if (infos->hasListener && callback != nullptr) {
        callback->notifyHandler = std::thread([infos] {
            GetNotifyEvent(infos);
            });
    }
}

int Copy::ExecCopy(std::shared_ptr<FileInfos> infos)
{
    if (IsFile(infos->srcPath) && IsFile(infos->destPath)) {
        // copyFile
        return CopyFile(infos->srcPath, infos->destPath, infos);
    }
    if (IsDirectory(infos->srcPath) && IsDirectory(infos->destPath)) {
        if (infos->srcPath.back() != '/') {
            infos->srcPath += '/';
        }
        if (infos->destPath.back() != '/') {
            infos->destPath += '/';
        }
        // copyDir
        return CopyDirFunc(infos->srcPath, infos->destPath, infos);
    }
    return EINVAL;
}

int Copy::ParseJsParam(napi_env env, NFuncArg &funcArg, std::shared_ptr<FileInfos> &fileInfos)
{
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        return E_PARAMS;
    }
    auto [succSrc, srcUri] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    auto [succDest, destUri] = ParseJsOperand(env, { env, funcArg[NARG_POS::SECOND] });
    auto [succListener, listener] = GetListenerFromOptionArg(env, funcArg);
    auto [succSignal, copySignal] = GetCopySignalFromOptionArg(env, funcArg);
    if (!succSrc || !succDest || !succListener || !succSignal) {
        HILOGE("The first/second/third argument requires uri/uri/napi_function");
        return E_PARAMS;
    }
    auto [errCode, infos] = CreateFileInfos(srcUri, destUri, listener, copySignal);
    if (errCode != ERRNO_NOERR) {
        return errCode;
    }
    fileInfos = infos;
    return ERRNO_NOERR;
}

void Copy::WaitNotifyFinished(std::shared_ptr<JsCallbackObject> callback)
{
    if (callback != nullptr) {
        if (callback->notifyHandler.joinable()) {
            callback->notifyHandler.join();
        }
    }
}

void Copy::CopyComplete(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    if (callback != nullptr && infos->hasListener) {
        callback->progressSize = callback->totalSize;
        OnFileReceive(infos);
    }
}

napi_value Copy::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    std::shared_ptr<FileInfos> infos = nullptr;
    auto result = ParseJsParam(env, funcArg, infos);
    if (result != ERRNO_NOERR) {
        NError(result).ThrowErr(env);
        return nullptr;
    }
    auto callback = RegisterListener(env, infos);
    if (callback == nullptr) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [infos, callback]() -> NError {
        if (IsRemoteUri(infos->srcUri)) {
            if (infos->taskSignal != nullptr) {
                infos->taskSignal->MarkRemoteTask();
            }
            auto ret = TransListener::CopyFileFromSoftBus(infos->srcUri, infos->destUri,
                                                          infos, std::move(callback));
            return ret;
        }
        auto result = Copy::ExecLocal(infos, callback);
        CloseNotifyFd(infos, callback);
        infos->run = false;
        WaitNotifyFinished(callback);
        if (result != ERRNO_NOERR) {
            infos->exceptionCode = result;
            return NError(infos->exceptionCode);
        }
        CopyComplete(infos, callback);
        return NError(infos->exceptionCode);
    };

    auto cbCompl = [infos](napi_env env, NError err) -> NVal {
        UnregisterListener(infos);
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO ||
        (funcArg.GetArgc() == NARG_CNT::THREE && !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_COPY_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_COPY_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS