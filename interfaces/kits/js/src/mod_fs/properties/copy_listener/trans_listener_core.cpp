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

#include "trans_listener_core.h"

#include <dirent.h>
#include <filesystem>
#include <random>

#include "ipc_skeleton.h"
#include "sandbox_helper.h"
#include "uri.h"
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
std::atomic<uint32_t> TransListenerCore::getSequenceId_ = 0;

void TransListenerCore::RmDir(const std::string &path)
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

std::string TransListenerCore::CreateDfsCopyPath()
{
    std::random_device rd;
    std::string random = std::to_string(rd());
    while (std::filesystem::exists(DISTRIBUTED_PATH + random)) {
        random = std::to_string(rd());
    }
    return random;
}

int TransListenerCore::HandleCopyFailure(CopyEvent &copyEvent, const Storage::DistributedFile::HmdfsInfo &info,
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
        return EIO;
    }
    if (copyEvent.errorCode != DFS_CANCEL_SUCCESS) {
        HILOGE("HandleCopyFailure failed, copyEvent.errorCode = %{public}d.", copyEvent.errorCode);
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::SEND_FILE_ERROR, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::PACKAGE_NAME, to_string(copyEvent.errorCode));
    }
    return it->second;
}

int TransListenerCore::WaitForCopyResult(TransListenerCore* transListener)
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

int TransListenerCore::CopyFileFromSoftBus(const std::string &srcUri, const std::string &destUri,
    std::shared_ptr<FileInfosCore> fileInfos, std::shared_ptr<CallbackObjectCore> callback)
{
    HILOGI("CopyFileFromSoftBus begin.");
    std::string currentId = "CopyFile_" + std::to_string(getpid()) + "_" + std::to_string(getSequenceId_);
    ++getSequenceId_;
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::PACKAGE_NAME, std::to_string(getpid()),
        RadarReporter::CONCURRENT_ID, currentId);
    sptr<TransListenerCore> transListener = new (std::nothrow) TransListenerCore();
    if (transListener == nullptr) {
        HILOGE("new trans listener failed");
        return ENOMEM;
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
        return EIO;
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
        return ERRNO_NOERR;
    }

    ret = CopyToSandBox(srcUri, disSandboxPath, info.sandboxPath, currentId);
    RmDir(disSandboxPath);
    if (ret != ERRNO_NOERR) {
        HILOGE("CopyToSandBox failed, ret = %{public}d.", ret);
        return EIO;
    }
    return ERRNO_NOERR;
}

int32_t TransListenerCore::PrepareCopySession(const std::string &srcUri,
                                              const std::string &destUri,
                                              TransListenerCore* transListener,
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

int32_t TransListenerCore::CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
    const std::string &sandboxPath, const std::string &currentId)
{
    std::error_code errCode;
    if (std::filesystem::exists(sandboxPath) && std::filesystem::is_directory(sandboxPath)) {
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

std::string TransListenerCore::GetFileName(const std::string &path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        HILOGE("invalid path");
        return "";
    }
    return SandboxHelper::Decode(path.substr(pos));
}

std::string TransListenerCore::GetNetworkIdFromUri(const std::string &uri)
{
    return uri.substr(uri.find(NETWORK_PARA) + NETWORK_PARA.size(), uri.size());
}

void TransListenerCore::CallbackComplete(std::shared_ptr<UvEntryCore> entry)
{
    if (entry == nullptr) {
        HILOGE("entry pointer is nullptr.");
        return;
    }

    entry->callback->listenerCb(entry->progressSize, entry->totalSize);
}

int32_t TransListenerCore::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (callback_ == nullptr) {
        HILOGE("Failed to parse watcher callback");
        return ENOMEM;
    }

    std::shared_ptr<UvEntryCore> entry = std::make_shared<UvEntryCore>(callback_);
    if (entry == nullptr) {
        HILOGE("entry ptr is nullptr");
        return ENOMEM;
    }
    entry->progressSize = processedBytes;
    entry->totalSize = totalBytes;
    CallbackComplete(entry);
    return ERRNO_NOERR;
}

int32_t TransListenerCore::OnFinished(const std::string &sessionName)
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

int32_t TransListenerCore::OnFailed(const std::string &sessionName, int32_t errorCode)
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