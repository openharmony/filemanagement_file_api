/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <sys/inotify.h>
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
const std::string FILE_PREFIX_NAME = "file://";
const std::string NETWORK_PARA = "?networkid=";
const string PROCEDURE_COPY_NAME = "FileFSCopy";
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;
constexpr int BUF_SIZE = 1024;
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

bool Copy::IsRemoteUri(const std::string &uri)
{
    // NETWORK_PARA
    return uri.find(NETWORK_PARA) != uri.npos;
}

bool Copy::IsDirectory(const std::string &path)
{
    struct stat buf {
    };
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGE("stat failed, errno is %{public}d, path is %{public}s", errno, path.c_str());
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

bool Copy::IsFile(const std::string &path)
{
    struct stat buf {
    };
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("stat failed, errno is %{public}d, ", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

tuple<int, uint64_t> Copy::GetFileSize(const std::string &path)
{
    struct stat buf {
    };
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        HILOGI("Stat failed.");
        return { errno, 0 };
    }
    return { ERRNO_NOERR, buf.st_size };
}

bool Copy::CheckValidParam(const std::string &srcUri, const std::string &destUri)
{
    auto srcPath = ConvertUriToPath(srcUri);
    auto destPath = ConvertUriToPath(destUri);
    if (!filesystem::exists(destPath)) {
        HILOGE("destPath not exist, destPath = %{public}s", destPath.c_str());
        ofstream out;
        out.open(destPath.c_str());
        out.close();
    }
    if (IsDirectory(srcPath)) {
        return IsDirectory(destPath);
    }
    if (IsFile(srcPath)) {
        return IsFile(destPath);
    }
    return false;
}

int Copy::CopyFile(const string &src, const string &dest)
{
    HILOGD("src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
    filesystem::path dstPath(dest);
    filesystem::path srcPath(src);
    std::error_code errCode;
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
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

uint64_t Copy::GetDirSize(std::shared_ptr<FileInfos> infos, std::string path)
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
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            return size += GetDirSize(infos, dest);
        }
        struct stat st {};
        if (stat(dest.c_str(), &st) == -1) {
            return size;
        }
        size += st.st_size;
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
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            return CopySubDir(src, dest, infos);
        }
        infos->filePaths.insert(dest);
        auto res = CopyFile(src, dest);
        if (res != ERRNO_NOERR) {
            return res;
        }
    }
    return ERRNO_NOERR;
}

int Copy::CopyDirFunc(const string &src, const string &dest, std::shared_ptr<FileInfos> infos)
{
    HILOGD("CopyDirFunc in, src = %{public}s, dest = %{public}s", src.c_str(), dest.c_str());
    size_t found = string(src).rfind('/');
    if (found == std::string::npos) {
        return EINVAL;
    }
    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    return CopySubDir(src, destStr, infos);
}

int Copy::SubscribeLocalListener(napi_env env, std::shared_ptr<FileInfos> infos)
{
    if (!infos->listener.TypeIs(napi_function)) {
        return ERRNO_NOERR;
    }
    infos->notifyFd = inotify_init();
    if (infos->notifyFd < 0) {
        HILOGE("Failed to init inotify, errno:%{public}d", errno);
        return errno;
    }
    uint32_t watchEvents = IN_MODIFY;
    int newWd = inotify_add_watch(infos->notifyFd, infos->destPath.c_str(), watchEvents);
    if (newWd < 0) {
        auto errNo = errno;
        HILOGE("Failed to add inotify watch, errno:%{public}d, notifyFd = %{public}d, destPath = %{public}s", errNo,
            infos->notifyFd, infos->destPath.c_str());
        close(infos->notifyFd);
        return errNo;
    }
    auto receiveInfo = CreateSharedPtr<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    receiveInfo->path = infos->destPath;
    auto callback = CreateSharedPtr<JsCallbackObject>(env, infos->listener);
    if (callback == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    callback->wds.push_back({ newWd, receiveInfo });
    if (IsFile(infos->srcPath)) {
        auto [err, fileSize] = GetFileSize(infos->srcPath);
        if (err != ERRNO_NOERR) {
            return err;
        }
        callback->totalSize = fileSize;
    } else {
        callback->totalSize = GetDirSize(infos, infos->srcPath);
    }
    if (RegisterListener(infos, callback)) {
        return ERRNO_NOERR;
    }
    inotify_rm_watch(infos->notifyFd, newWd);
    close(infos->notifyFd);
    HILOGE("Can not copy repeat.");
    return EINVAL;
}

bool Copy::RegisterListener(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = jsCbMap_.find(*infos);
    if (iter != jsCbMap_.end()) {
        HILOGI("Copy::RegisterListener, already registered.");
        return false;
    }
    jsCbMap_.insert({ *infos, callback });
    return true;
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
    HILOGD("UnregisterListener end.");
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
    if (processedSize <= entry->callback->maxProgressSize) {
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
    if (processedSize <= numeric_limits<int64_t >::max() && entry->totalSize <= numeric_limits<int64_t >::max()) {
        obj.AddProp("processedSize", NVal::CreateInt64(env, processedSize).val_);
        obj.AddProp("totalSize", NVal::CreateInt64(env, entry->totalSize).val_);
    }
    napi_value result = nullptr;
    napi_value jsCallback = entry->callback->nRef.Deref(env).val_;
    status = napi_call_function(env, nullptr, jsCallback, 1, &(obj.val_), &result);
    if (status != napi_ok) {
        HILOGE("Failed to get result, status: %{public}d.", status);
    }
    if (entry->progressSize == entry->totalSize) {
        HILOGI("entry->progressSize == entry->totalSize, %" PRId64, entry->progressSize);
        UnregisterListener(entry->fileInfos);
    }
    status = napi_close_handle_scope(env, scope);
    if (status != napi_ok) {
        HILOGE("Failed to close scope, status: %{public}d.", status);
    }
}

void Copy::UnregisterListenerComplete(uv_work_t *work, int stat)
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
    UnregisterListener(entry->fileInfos);
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
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(ReceiveComplete));
}

void Copy::OnUnregisterListener(std::shared_ptr<FileInfos> infos)
{
    uv_work_t *work = GetUVwork(infos);
    if (work == nullptr) {
        HILOGE("Failed to create uv_work_t pointer");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(infos->env, &loop);
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(UnregisterListenerComplete));
}

fd_set Copy::InitFds(int notifyFd)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(notifyFd, &fds);
    return fds;
}

std::shared_ptr<ReceiveInfo> Copy::GetReceivedInfo(int wd, std::shared_ptr<JsCallbackObject> callback)
{
    std::string path;
    auto wds = callback->wds;
    auto it = wds.begin();
    for (; it != wds.end(); ++it) {
        if (it->first == wd) {
            return it->second;
        }
    }
    return nullptr;
}

bool Copy::CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfos> infos)
{
    return infos->filePaths.count(filePath) != 0;
}

int Copy::UpdateProgressSize(const std::string &filePath, std::shared_ptr<FileInfos> infos,
    std::shared_ptr<ReceiveInfo> receivedInfo, std::shared_ptr<JsCallbackObject> callback)
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
        auto err = UpdateProgressSize(fileName, infos, receivedInfo, callback);
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

void Copy::RemoveWatch(int notifyFd, std::shared_ptr<JsCallbackObject> callback)
{
    for (auto item : callback->wds) {
        inotify_rm_watch(notifyFd, item.first);
    }
    close(notifyFd);
}

void Copy::GetNotifyEvent(std::shared_ptr<FileInfos> infos)
{
    if (infos->run || infos->notifyFd < 0) {
        HILOGE("Already run or notifyFd is invalid, notifyFd: %{public}d.", infos->notifyFd);
        infos->exceptionCode = EINVAL;
        return;
    }
    infos->run = true;
    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *event = nullptr;
    fd_set fds = InitFds(infos->notifyFd);
    while (infos->run && infos->exceptionCode == ERRNO_NOERR) {
        auto ret = select(infos->notifyFd + 1, &fds, nullptr, nullptr, nullptr);
        if (ret <= 0) {
            HILOGD("Failed to select, ret = %{public}d.", ret);
            infos->exceptionCode = errno;
            break;
        }
        int len, index = 0;
        while (((len = read(infos->notifyFd, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {};
        while (index < len) {
            event = reinterpret_cast<inotify_event *>(buf + index);
            auto callback = GetRegisteredListener(infos);
            if (callback == nullptr) {
                infos->exceptionCode = EINVAL;
                infos->run = false;
                return;
            }
            auto [needContinue, errCode, needSend] = HandleProgress(event, infos, callback);
            if (!needContinue) {
                infos->exceptionCode = errCode;
                RemoveWatch(infos->notifyFd, callback);
                callback = nullptr;
                OnUnregisterListener(infos);
                return;
            }
            if (needContinue && !needSend) {
                index += sizeof(struct inotify_event) + event->len;
                continue;
            }
            if (callback->progressSize == callback->totalSize) {
                infos->run = false;
                RemoveWatch(infos->notifyFd, callback);
            }
            callback = nullptr;
            OnFileReceive(infos);
            index += sizeof(struct inotify_event) + event->len;
        }
    }
    OnUnregisterListener(infos);
}

std::string Copy::ConvertUriToPath(const std::string &uri)
{
    FileUri fileUri(uri);
    return fileUri.GetPath();
}

tuple<int, std::shared_ptr<FileInfos>> Copy::CreateFileInfos(
    const std::string &srcUri, const std::string &destUri, NVal &listener)
{
    auto infos = CreateSharedPtr<FileInfos>();
    if (infos == nullptr) {
        HILOGE("Failed to request heap memory.");
        return { ENOMEM, nullptr };
    }
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    infos->listener = listener;
    infos->srcPath = ConvertUriToPath(infos->srcUri);
    infos->destPath = ConvertUriToPath(infos->destUri);
    return { ERRNO_NOERR, infos };
}

void Copy::StartNotify(std::shared_ptr<FileInfos> infos)
{
    if (infos->notifyFd != -1) {
        std::thread([infos] {
            GetNotifyEvent(infos);
            }).detach();
    }
}

int Copy::ExecCopy(std::shared_ptr<FileInfos> infos)
{
    if (IsFile(infos->srcPath) && IsFile(infos->destPath)) {
        // copyFile
        return CopyFile(infos->srcPath.c_str(), infos->destPath.c_str());
    }
    if (IsDirectory(infos->srcPath) && IsDirectory(infos->destPath)) {
        // copyDir
        return CopyDirFunc(infos->srcPath.c_str(), infos->destPath.c_str(), infos);
    }
    return EINVAL;
}

int Copy::ParseJsParam(napi_env env, NFuncArg &funcArg, std::shared_ptr<FileInfos> &fileInfos)
{
    auto [succSrc, srcUri] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    auto [succDest, destUri] = ParseJsOperand(env, { env, funcArg[NARG_POS::SECOND] });
    auto [succOptions, listener] = GetListenerFromOptionArg(env, funcArg);
    if (!succSrc || !succDest || !succOptions) {
        HILOGE("The first/second/third argument requires uri/uri/napi_function");
        return E_PARAMS;
    }

    if (!IsRemoteUri(srcUri) && !CheckValidParam(srcUri, destUri)) {
        HILOGE("Should copy one file to another, or copy one dir to another.");
        return E_PARAMS;
    }
    auto [errCode, infos] = CreateFileInfos(srcUri, destUri, listener);
    if (errCode != ERRNO_NOERR) {
        return errCode;
    }
    fileInfos = infos;
    return ERRNO_NOERR;
}

napi_value Copy::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::shared_ptr<FileInfos> infos = nullptr;
    auto result = ParseJsParam(env, funcArg, infos);
    if (result != ERRNO_NOERR) {
        NError(result).ThrowErr(env);
        return nullptr;
    }
    if (!IsRemoteUri(infos->srcUri)) {
        auto ret = SubscribeLocalListener(env, infos);
        if (ret != ERRNO_NOERR) {
            NError(ret).ThrowErr(env);
            return nullptr;
        }
    }
    std::shared_ptr<FileInfos> tempInfos = infos;
    auto cbExec = [env, tempInfos]() -> NError {
        if (IsRemoteUri(tempInfos->srcUri)) {
            // copyRemoteUri
            return SubscribeRemoteListener(env, tempInfos);
        }
        StartNotify(tempInfos);
        auto result = ExecCopy(tempInfos);
        if (result != ERRNO_NOERR) {
            tempInfos->exceptionCode = result;
        }
        return NError(tempInfos->exceptionCode);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
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

NError Copy::SubscribeRemoteListener(napi_env env, std::shared_ptr<FileInfos> infos)
{
    sptr<TransListener> transListener = new (std::nothrow) TransListener;
    if (transListener == nullptr) {
        HILOGE("transListener is empty");
        return NError(ENOMEM);
    }
    transListener->callback_ = std::make_shared<JsCallbackObject>(env, infos->listener);
    return TransListener::CopyFileFromSoftBus(infos->srcUri, infos->destUri, transListener);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS