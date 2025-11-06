/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "copy/file_copy_manager.h"
#include "datashare_helper.h"
#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "trans_listener.h"
#include "utils_log.h"
#include "common_func.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace AppFileService::ModuleFileUri;
namespace fs = std::filesystem;
const std::string FILE_PREFIX_NAME = "file://";
const std::string NETWORK_PARA = "?networkid=";
const string PROCEDURE_COPY_NAME = "FileFSCopy";
const std::string MEDIA = "media";
constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(300);
std::recursive_mutex Copy::mutex_;
std::map<FileInfos, std::shared_ptr<JsCallbackObject>> Copy::jsCbMap_;
uint32_t g_apiCompatibleVersion = 0;

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
        HILOGE("stat failed, errno is %{public}d", errno);
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

bool Copy::IsMediaUri(const std::string &uriPath)
{
    Uri uri(uriPath);
    string bundleName = uri.GetAuthority();
    return bundleName == MEDIA;
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

struct NameList {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

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

void Copy::UnregisterListener(std::shared_ptr<FileInfos> fileInfos)
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

void Copy::ReceiveComplete(std::shared_ptr<UvEntry> entry)
{
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
    if (status != napi_ok || scope == nullptr) {
        HILOGE("Failed to open handle scope, status: %{public}d.", status);
        return;
    }
    NVal obj = NVal::CreateObject(env);
    if (processedSize <= MAX_VALUE && entry->totalSize <= MAX_VALUE) {
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

UvEntry *Copy::GetUVwork(std::shared_ptr<FileInfos> infos)
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

    return entry;
}

void Copy::OnFileReceive(std::shared_ptr<FileInfos> infos)
{
    std::shared_ptr<UvEntry> entry(GetUVwork(infos));
    auto task = [entry] () {
        ReceiveComplete(entry);
    };
    auto ret = napi_send_event(infos->env, task, napi_eprio_immediate, "file_api_copy_onFileReceive");
    if (ret != 0) {
        HILOGE("Failed to call napi_send_event");
    }
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

tuple<bool, std::string> Copy::ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg)
{
    auto [succ, uri, ignore] = pathOrFdFromJsArg.ToUTF8StringPath();
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

bool Copy::IsValidUri(const std::string &uri)
{
    return uri.find(FILE_PREFIX_NAME) == 0;
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

tuple<int, std::shared_ptr<FileInfos>> Copy::CreateFileInfos(
    const std::string &srcUri, const std::string &destUri, NVal &listener, NVal copySignal)
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
    FileUri srcFileUri(infos->srcUri);
    infos->srcPath = srcFileUri.GetRealPath();
    FileUri dstFileUri(infos->destUri);
    infos->destPath = dstFileUri.GetPath();
    infos->srcPath = GetRealPath(infos->srcPath);
    infos->destPath = GetRealPath(infos->destPath);
    infos->isFile = IsMediaUri(infos->srcUri) || IsFile(infos->srcPath);
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

void Copy::CopyComplete(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    if (callback != nullptr && infos->hasListener) {
        callback->progressSize = callback->totalSize;
        OnFileReceive(infos);
    }
}

int32_t Copy::ExecCopy(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback)
{
    Storage::DistributedFile::ProcessCallback processListener;
    if (infos->hasListener) {
        processListener = [infos, callback](uint64_t processSize, uint64_t totalSize) -> void {
            callback->progressSize = processSize;
            callback->totalSize = totalSize;
            if (processSize != totalSize) {
                OnFileReceive(infos);
            }
        };
    }
    if (infos->taskSignal != nullptr) {
        infos->taskSignal->MarkDfsTask();
        infos->taskSignal->SetCopyTaskUri(infos->srcUri, infos->destUri);
    }
    LOGI("Copy begin");
    auto result = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(infos->srcUri, infos->destUri,
        processListener);
    LOGI("Copy end, result = %{public}d", result);
    return result;
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
        auto result = ExecCopy(infos, callback);
        auto it = softbusErr2ErrCodeTable.find(result);
        if (it != softbusErr2ErrCodeTable.end()) {
            result = it->second;
        }
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
        return NAsyncWorkCallback(env, thisVar, cb, "file_api_copy")
            .Schedule(PROCEDURE_COPY_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS