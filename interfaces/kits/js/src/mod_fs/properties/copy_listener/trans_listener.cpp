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

#include "trans_listener.h"

#include <dirent.h>
#include <filesystem>
#include <random>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "sandbox_helper.h"
#include "uri.h"
#include "n_error.h"
#include "dfs_event_dfx.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::AppFileService;
using namespace AppFileService::ModuleFileUri;
using HapTokenInfo = OHOS::Security::AccessToken::HapTokenInfo;
using AccessTokenKit = OHOS::Security::AccessToken::AccessTokenKit;
const std::string NETWORK_PARA = "?networkid=";
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles/";
std::atomic<uint32_t> TransListener::getSequenceId_ = 0;

void TransListener::RmDir(const std::string &path)
{
    HILOGI("RmDirm path : %{public}s", path.c_str());
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
    while (std::filesystem::exists(DISTRIBUTED_PATH + random)) {
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
    auto it = softbusErr2ErrCodeTable.find(transListener->copyEvent_.errorCode);
    if (it == softbusErr2ErrCodeTable.end()) {
        RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::SEND_FILE_ERROR, RadarReporter::CONCURRENT_ID, currentId);
        return NError(EIO);
    }
    RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::SEND_FILE_ERROR, RadarReporter::CONCURRENT_ID, currentId);
    return NError(it->second);
}

int TransListener::WaitForCopyResult(TransListener* transListener)
{
    if (transListener == nullptr) {
        HILOGE("transListener is nullptr.");
        return FAILED;
    }
    std::unique_lock<std::mutex> lock(transListener->cvMutex_);
    transListener->cv_.wait(lock, [&transListener]() {
            return transListener->copyEvent_.copyResult == SUCCESS ||
                transListener->copyEvent_.copyResult == FAILED;
    });
    return transListener->copyEvent_.copyResult;
}

TransListener::CopyFileFromSoftBus(const std::string &srcUri, const std::string &destUri,
    std::shared_ptr<FileInfos> fileInfos, std::shared_ptr<JsCallbackObject> callback)
{
    HILOGI("CopyFileFromSoftBus begin.");
    std::string currentId = "GetPastData_" + std::string(getpid() + "_" + std::to_string(getSequenceId_));
    ++getSequenceId_;
    HapTokenInfo hapTokenInfo;
    int result = AccessTokenKit::GetHapTokenInfo(IPCSelfTokenID(), hapTokenInfo);
    if (result != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        HILOGE("GetHapTokenInfo failed, errCode = %{public}d", result);
        return NError(EIO);
    }
    RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::PACKAGE_NAME, hapTokenInfo.bundleName,
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
        RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::PERPARE_COPY_SESSION_ERROR, RadarReporter::CONCURRENT_ID, currentId);
        HILOGE("PrepareCopySession failed, ret = %{public}d.", ret);
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
        RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
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
    if (std::filesystem::exists(sandboxPath) && std::filesystem::is_directory(sandboxPath)) {
        HILOGI("Copy dir");
        std::filesystem::copy(disSandboxPath, sandboxPath,
            std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            HILOGE("Copy dir failed: errCode: %{public}d", errCode.value());
            RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                RadarReporter::COPY_TO_SANDBOX_ERROR, RadarReporter::CONCURRENT_ID, currentId);
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
            RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
                RadarReporter::COPY_TO_SANDBOX_ERROR, RadarReporter::CONCURRENT_ID, currentId);
            return EIO;
        }
    }
    HILOGI("Copy file success.");
    RADAR_REPORT(RadarReproter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
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

void TransListener::CallbackComplete(uv_work_t *work, int stat)
{
    if (work == nullptr) {
        HILOGE("Failed to get uv_queue_work pointer");
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
    napi_handle_scope scope = nullptr;
    napi_env env = entry->callback->env;
    napi_status status = napi_open_handle_scope(env, &scope);
    if (status != napi_ok) {
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

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback_->env, &loop);
    if (loop == nullptr) {
        HILOGE("Failed to get uv event loop");
        return ENOMEM;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("Failed to create uv_work_t pointer");
        return ENOMEM;
    }

    UvEntry *entry = new (std::nothrow) UvEntry(callback_);
    if (entry == nullptr) {
        HILOGE("entry ptr is nullptr");
        delete work;
        return ENOMEM;
    }
    entry->progressSize = processedBytes;
    entry->totalSize = totalBytes;
    work->data = entry;
    int retVal = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(CallbackComplete));
    if (retVal != 0) {
        HILOGE("failed to get uv_queue_work");
        delete (reinterpret_cast<UvEntry *>(work->data));
        delete work;
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
    copyEvent_.copyResult = SUCCESS;
    cv_.notify_all();
    return ERRNO_NOERR;
}

int32_t TransListener::OnFailed(const std::string &sessionName, int32_t errorCode)
{
    HILOGI("OnFailed, errorCode is %{public}d", errorCode);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    copyEvent_.copyResult = FAILED;
    copyEvent_.errorCode = errorCode;
    cv_.notify_all();
    return ERRNO_NOERR;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS