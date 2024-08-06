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

#include "translistener.h"
#include <filesystem>
#include <random>
#include <thread>
#include "sandbox_helper.h"
#include "uri.h"
#include "uni_error.h"

using Uri = OHOS::Uri;
namespace OHOS {
namespace CJSystemapi {
using namespace FileFs;
const std::string NETWORK_PARA = "?networkid=";
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles/";

void TransListener::RmDir(const std::string &path)
{
    LOGI("RmDirm path : %{public}s", path.c_str());
    std::filesystem::path pathName(path);
    std::error_code errCode;
    if (std::filesystem::exists(pathName, errCode)) {
        std::filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            LOGE("Failed to remove directory, error code: %{public}d", errCode.value());
        }
    } else {
        LOGE("pathName is not exists, error code: %{public}d", errCode.value());
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

int32_t TransListener::CopyFileFromSoftBus(const std::string& srcUri, const std::string& destUri,
    std::shared_ptr<FileInfos> fileInfos, std::shared_ptr<CjCallbackObject> callback)
{
    LOGI("CopyFileFromSoftBus begin.");
    sptr<TransListener> transListener = new (std::nothrow) TransListener();
    if (transListener == nullptr) {
        LOGE("new trans listener failed");
        return ENOMEM;
    }
    transListener->callback_ = std::move(callback);
    Storage::DistributedFile::HmdfsInfo info{};
    Uri uri(destUri);
    info.authority = uri.GetAuthority();
    info.sandboxPath = AppFileService::SandboxHelper::Decode(uri.GetPath());
    std::string disSandboxPath;
    auto ret = PrepareCopySession(srcUri, destUri, transListener, info, disSandboxPath);
    if (ret != 0) {
        LOGE("PrepareCopySession failed, ret = %{public}d.", ret);
        return EIO;
    }
    if (fileInfos->taskSignal != nullptr) {
        fileInfos->taskSignal->SetFileInfoOfRemoteTask(info.sessionName, fileInfos->srcPath);
    }
    std::unique_lock<std::mutex> lock(transListener->cvMutex_);
    transListener->cv_.wait(lock, [&transListener]() {
        return transListener->copyEvent_.copyResult == SUCCESS ||
            transListener->copyEvent_.copyResult == FAILED;
    });
    LOGI("dfs PrepareSession Finish, result is %{public}d", transListener->copyEvent_.copyResult);
    if (transListener->copyEvent_.copyResult == FAILED) {
        if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
            RmDir(disSandboxPath);
        }
        auto it = softbusErr2ErrCodeTable.find(transListener->copyEvent_.errorCode);
        if (it == softbusErr2ErrCodeTable.end()) {
            return EIO;
        }
        return it->second;
    }
    if (info.authority == FILE_MANAGER_AUTHORITY || info.authority == MEDIA_AUTHORITY) {
        LOGE("Public or media path not copy");
        return 0;
    }
    ret = CopyToSandBox(srcUri, disSandboxPath, info.sandboxPath);
    RmDir(disSandboxPath);
    if (ret != 0) {
        LOGE("CopyToSandBox failed, ret = %{public}d.", ret);
        return EIO;
    }
    return 0;
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
            LOGE("Create dir failed, error code: %{public}d", errCode.value());
            return errCode.value();
        }

        auto pos = info.sandboxPath.rfind('/');
        if (pos == std::string::npos) {
            LOGE("invalid file path");
            return EIO;
        }
        auto sandboxDir = info.sandboxPath.substr(0, pos);
        if (std::filesystem::exists(sandboxDir, errCode)) {
            info.dirExistFlag = true;
        }
    }

    info.copyPath = tmpDir;
    auto networkId = GetNetworkIdFromUri(srcUri);
    LOGI("dfs PrepareSession begin.");
    auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().PrepareSession(srcUri, destUri,
        networkId, transListener, info);
    if (ret != ERRNO_NOERR) {
        LOGE("PrepareSession failed, ret = %{public}d.", ret);
        if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
            RmDir(disSandboxPath);
        }
        return EIO;
    }
    return ERRNO_NOERR;
}

int32_t TransListener::CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
    const std::string &sandboxPath)
{
    std::error_code errCode;
    if (std::filesystem::exists(sandboxPath) && std::filesystem::is_directory(sandboxPath)) {
        LOGI("Copy dir");
        std::filesystem::copy(disSandboxPath, sandboxPath,
            std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            LOGE("Copy dir failed: errCode: %{public}d", errCode.value());
            return EIO;
        }
    } else {
        LOGI("Copy file.");
        Uri uri(srcUri);
        auto fileName = GetFileName(uri.GetPath());
        if (fileName.empty()) {
            LOGE("Get filename failed");
            RmDir(disSandboxPath);
            return EIO;
        }
        std::filesystem::copy(disSandboxPath + fileName, sandboxPath, std::filesystem::copy_options::update_existing,
            errCode);
        if (errCode.value() != 0) {
            LOGE("Copy file failed: errCode: %{public}d", errCode.value());
            return EIO;
        }
    }
    LOGI("Copy file success.");
    return ERRNO_NOERR;
}

std::string TransListener::GetFileName(const std::string &path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        LOGE("invalid path");
        return "";
    }
    return AppFileService::SandboxHelper::Decode(path.substr(pos));
}

std::string TransListener::GetNetworkIdFromUri(const std::string &uri)
{
    return uri.substr(uri.find(NETWORK_PARA) + NETWORK_PARA.size(), uri.size());
}

void TransListener::CallbackComplete(TransListener* transListener, CProgress progress)
{
    if (transListener == nullptr ||
        transListener->callback_ == nullptr ||
        transListener->callback_->callback == nullptr) {
        LOGE("Failed to get copy progress callback.");
        return;
    }
    transListener->callback_->callback(progress);
}

int32_t TransListener::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (callback_ == nullptr) {
        LOGE("Failed to parse watcher callback");
        return ENOMEM;
    }
    CProgress progress = { .processedSize = processedBytes, .totalSize = totalBytes };
    std::thread([this, progress]() {
        CallbackComplete(this, progress);
    }).detach();
    return ERRNO_NOERR;
}

int32_t TransListener::OnFinished(const std::string &sessionName)
{
    LOGI("OnFinished");
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
    LOGI("OnFailed, errorCode is %{public}d", errorCode);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    copyEvent_.copyResult = FAILED;
    copyEvent_.errorCode = errorCode;
    cv_.notify_all();
    return ERRNO_NOERR;
}
}
}
