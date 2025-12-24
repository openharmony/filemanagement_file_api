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

#include "trans_listener.h"

#include <dirent.h>
#include <filesystem>
#include <random>

#include "ipc_skeleton.h"
#include "sandbox_helper.h"
#include "uri.h"
#include "n_error.h"
#include "dfs_event_dfx.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::AppFileService;
using namespace AppFileService::ModuleFileUri;
const std::string NETWORK_PARA = "?networkid=";
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles/";
std::atomic<uint32_t> TransListener::getSequenceId_ = 0;

void TransListener::RmDir(const std::string &path)
{
    HILOGI("RmDirm path : %{public}s", GetAnonyString(path).c_str());
    std::filesystem::path pathName(path);
    std::error_code errCode;
    if (std::filesystem::exists(pathName, errCode)) {
        std::filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            HILOGE("Failed to remove directory, error code: %{public}d", errCode.value());
        }
    } else {
        HILOGE("pathName is not exists, error code: %{public}d", errCode.value());
    }
}

std::string TransListener::CreateDfsCopyPath()
{
    std::random_device rd;
    std::string random = std::to_string(rd());
    std::error_code errCode;
    while (std::filesystem::exists(DISTRIBUTED_PATH + random, errCode)) {
        random = std::to_string(rd());
    }
    return random;
}

NError TransListener::HandleCopyFailure(CopyEvent &copyEvent, const Storage::DistributedFile::HmdfsInfo &info,
    const std::string &disSandboxPath, const std::string &currentId)
{
    if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
        RmDir(disSandboxPath);
    }
    auto it = softbusErr2ErrCodeTable.find(copyEvent.errorCode);
    if (it == softbusErr2ErrCodeTable.end()) {
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::SEND_FILE_ERROR, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::PACKAGE_NAME, to_string(copyEvent.errorCode));
        return NError(EIO);
    }
    if (copyEvent.errorCode != DFS_CANCEL_SUCCESS) {
        HILOGE("HandleCopyFailure failed, copyEvent.errorCode = %{public}d.", copyEvent.errorCode);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::SEND_FILE_ERROR, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::PACKAGE_NAME, to_string(copyEvent.errorCode));
    }
    return NError(it->second);
}

int TransListener::WaitForCopyResult(TransListener* transListener)
{
    if (transListener == nullptr) {
        HILOGE("transListener is nullptr");
        return FAILED;
    }
    std::unique_lock<std::mutex> lock(transListener->cvMutex_);
    transListener->cv_.wait(lock, [&transListener]() {
            return transListener->copyEvent_.copyResult == SUCCESS ||
                transListener->copyEvent_.copyResult == FAILED;
    });
    return transListener->copyEvent_.copyResult;
}

NError TransListener::CopyFileFromSoftBus(const std::string &srcUri, const std::string &destUri,
    std::shared_ptr<FileInfos> fileInfos, std::shared_ptr<JsCallbackObject> callback)
{
    HILOGI("CopyFileFromSoftBus begin.");
    std::string currentId = "CopyFile_" + std::to_string(getpid()) + "_" + std::to_string(getSequenceId_);
    ++getSequenceId_;
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::PACKAGE_NAME, std::to_string(getpid()),
        RadarReporter::CONCURRENT_ID, currentId);
    sptr<TransListener> transListener = new (std::nothrow) TransListener();
    if (transListener == nullptr) {
        HILOGE("new trans listener failed");
        return NError(ENOMEM);
    }
    transListener->callback_ = std::move(callback);

    Storage::DistributedFile::HmdfsInfo info{};
    Uri uri(destUri);
    info.authority = uri.GetAuthority();
    info.sandboxPath = SandboxHelper::Decode(uri.GetPath());
    std::string disSandboxPath;
    auto ret = PrepareCopySession(srcUri, destUri, transListener, info, disSandboxPath);
    if (ret != ERRNO_NOERR) {
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::PREPARE_COPY_SESSION_ERROR, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::PACKAGE_NAME, to_string(ret));
        return NError(EIO);
    }
    if (fileInfos->taskSignal != nullptr) {
        fileInfos->taskSignal->SetFileInfoOfRemoteTask(info.sessionName, fileInfos->srcPath);
    }
    auto copyResult = WaitForCopyResult(transListener);
    if (copyResult == FAILED) {
        return HandleCopyFailure(transListener->copyEvent_, info, disSandboxPath, currentId);
    }
    if (info.authority == FILE_MANAGER_AUTHORITY || info.authority == MEDIA_AUTHORITY) {
        HILOGW("Public or media path not copy");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId);
        return NError(ERRNO_NOERR);
    }

    ret = CopyToSandBox(srcUri, disSandboxPath, info.sandboxPath, currentId);
    RmDir(disSandboxPath);
    if (ret != ERRNO_NOERR) {
        HILOGE("CopyToSandBox failed, ret = %{public}d.", ret);
        return NError(EIO);
    }
    return NError(ERRNO_NOERR);
}

int32_t TransListener::PrepareCopySession(const std::string &srcUri,
                                          const std::string &destUri,
                                          TransListener* transListener,
                                          Storage::DistributedFile::HmdfsInfo &info,
                                          std::string &disSandboxPath)
{
    std::string tmpDir;
    if (info.authority != FILE_MANAGER_AUTHORITY && info.authority  != MEDIA_AUTHORITY) {
        tmpDir = CreateDfsCopyPath();
        disSandboxPath = DISTRIBUTED_PATH + tmpDir;
        std::error_code errCode;
        if (!std::filesystem::create_directory(disSandboxPath, errCode)) {
            HILOGE("Create dir failed, error code: %{public}d", errCode.value());
            return errCode.value();
        }

        auto pos = info.sandboxPath.rfind('/');
        if (pos == std::string::npos) {
            HILOGE("invalid file path");
            return EIO;
        }
        auto sandboxDir = info.sandboxPath.substr(0, pos);
        if (std::filesystem::exists(sandboxDir, errCode)) {
            info.dirExistFlag = true;
        }
    }

    info.copyPath = tmpDir;
    auto networkId = GetNetworkIdFromUri(srcUri);
    HILOGI("dfs PrepareSession begin.");
    auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().PrepareSession(srcUri, destUri,
        networkId, transListener, info);
    if (ret != ERRNO_NOERR) {
        HILOGE("PrepareSession failed, ret = %{public}d.", ret);
        if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
            RmDir(disSandboxPath);
        }
        return EIO;
    }
    return ERRNO_NOERR;
}

int32_t TransListener::CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
    const std::string &sandboxPath, const std::string &currentId)
{
    std::error_code errCode;
    std::error_code existErrCode;
    std::error_code isdirErrCode;
    bool isSandboxExist = std::filesystem::exists(sandboxPath, existErrCode);
    bool isSandboxDir = std::filesystem::is_directory(sandboxPath, isdirErrCode);

    if (existErrCode.value() != 0 || isdirErrCode.value() != 0) {
        int errValue = existErrCode ? existErrCode.value() : isdirErrCode.value();
        HILOGE("Copy dir failed: errCode: %{public}d", errValue);
            RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                RadarReporter::COPY_TO_SANDBOX_ERROR, RadarReporter::CONCURRENT_ID, currentId,
                RadarReporter::PACKAGE_NAME, to_string(errValue));
        return EIO;
    }

    if (isSandboxExist && isSandboxDir) {
        HILOGI("Copy dir");
        std::filesystem::copy(disSandboxPath, sandboxPath,
            std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            HILOGE("Copy dir failed: errCode: %{public}d", errCode.value());
            RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                RadarReporter::COPY_TO_SANDBOX_ERROR, RadarReporter::CONCURRENT_ID, currentId,
                RadarReporter::PACKAGE_NAME, to_string(errCode.value()));
            return EIO;
        }
    } else {
        HILOGI("Copy file.");
        Uri uri(srcUri);
        auto fileName = GetFileName(uri.GetPath());
        if (fileName.empty()) {
            HILOGE("Get filename failed");
            RmDir(disSandboxPath);
            return EIO;
        }
        std::filesystem::copy(disSandboxPath + fileName, sandboxPath, std::filesystem::copy_options::update_existing,
            errCode);
        if (errCode.value() != 0) {
            HILOGE("Copy file failed: errCode: %{public}d", errCode.value());
            RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                RadarReporter::COPY_TO_SANDBOX_ERROR, RadarReporter::CONCURRENT_ID, currentId,
                RadarReporter::PACKAGE_NAME, to_string(errCode.value()));
            return EIO;
        }
    }
    HILOGI("Copy file success.");
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId);
    return ERRNO_NOERR;
}

std::string TransListener::GetFileName(const std::string &path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        HILOGE("invalid path");
        return "";
    }
    return SandboxHelper::Decode(path.substr(pos));
}

std::string TransListener::GetNetworkIdFromUri(const std::string &uri)
{
    return uri.substr(uri.find(NETWORK_PARA) + NETWORK_PARA.size(), uri.size());
}

void TransListener::CallbackComplete(std::shared_ptr<UvEntry> entry)
{
    if (entry == nullptr) {
        HILOGE("entry pointer is nullptr.");
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_env env = entry->callback->env;
    napi_status status = napi_open_handle_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        HILOGE("Failed to open handle scope, status: %{public}d.", status);
        return;
    }
    NVal obj = NVal::CreateObject(env);
    if (entry->progressSize <= MAX_VALUE && entry->totalSize <= MAX_VALUE) {
        obj.AddProp("processedSize", NVal::CreateInt64(env, entry->progressSize).val_);
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

int32_t TransListener::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (callback_ == nullptr) {
        HILOGE("Failed to parse watcher callback");
        return ENOMEM;
    }

    std::shared_ptr<UvEntry> entry = std::make_shared<UvEntry>(callback_);
    if (entry == nullptr) {
        HILOGE("entry ptr is nullptr");
        return ENOMEM;
    }
    entry->progressSize = processedBytes;
    entry->totalSize = totalBytes;
    auto env = entry->callback->env;
    auto task = [entry] () {
        CallbackComplete(entry);
    };
    auto retVal = napi_send_event(env, task, napi_eprio_immediate, "fs.copy.ProgressListener");
    if (retVal != 0) {
        HILOGE("Failed to call napi_send_event");
        return ENOMEM;
    }
    return ERRNO_NOERR;
}

int32_t TransListener::OnFinished(const std::string &sessionName)
{
    HILOGI("OnFinished");
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        copyEvent_.copyResult = SUCCESS;
        cv_.notify_all();
    }
    return ERRNO_NOERR;
}

int32_t TransListener::OnFailed(const std::string &sessionName, int32_t errorCode)
{
    HILOGI("OnFailed, errorCode is %{public}d", errorCode);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        copyEvent_.copyResult = FAILED;
        copyEvent_.errorCode = errorCode;
        cv_.notify_all();
    }
    return ERRNO_NOERR;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS