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

#include "distributed_file_daemon_manager.h"
#include "file_uri.h"
#include "ipc_skeleton.h"
#include "uri.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::AppFileService;
using namespace AppFileService::ModuleFileUri;
const std::string NETWORK_PARA = "?networkid=";
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles";

void TransListener::RmDirectory(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        HILOGE("Open dir failed");
        return;
    }
    dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, ".remote_share") == 0) {
            continue;
        }
        std::string subPath = path + "/" + entry->d_name;
        std::filesystem::path pathName(subPath);
        if (std::filesystem::exists(pathName)) {
            std::error_code errCode;
            std::filesystem::remove_all(pathName, errCode);
            if (errCode.value() != 0) {
                closedir(dir);
                HILOGE("Failed to remove directory, error code: %{public}d", errCode.value());
                return;
            }
        }
    }
    closedir(dir);
}

void TransListener::CopyDir(const std::string &path, const std::string &sandboxPath)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        HILOGE("Open dir failed");
        return;
    }
    dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, ".remote_share") == 0) {
            continue;
        }
        std::string subPath = path + "/" + entry->d_name;
        if (std::filesystem::is_directory(subPath)) {
            auto pos = subPath.find_last_of('/');
            if (pos == std::string::npos) {
                closedir(dir);
                return;
            }
            auto dirName = subPath.substr(pos);
            std::filesystem::create_directories(sandboxPath + dirName);
            std::filesystem::copy(subPath, sandboxPath + dirName,
                std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        } else {
            std::filesystem::copy(subPath, sandboxPath, std::filesystem::copy_options::update_existing);
        }
    }
    closedir(dir);
}

NError TransListener::CopyFileFromSoftBus(const std::string &srcUri,
                                          const std::string &destUri,
                                          std::shared_ptr<JsCallbackObject> callback)
{
    sptr<TransListener> transListener = new (std::nothrow) TransListener();
    if (transListener == nullptr) {
        HILOGE("new trans listener failed");
        return NError(ENOMEM);
    }
    transListener->callback_ = std::move(callback);
    auto networkId = GetNetworkIdFromUri(srcUri);
    auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().PrepareSession(srcUri, destUri,
        networkId, transListener);
    if (ret != ERRNO_NOERR) {
        HILOGE("PrepareSession failed, ret = %{public}d.", ret);
        return NError(ret);
    }
    std::unique_lock<std::mutex> lock(transListener->cvMutex_);
    transListener->cv_.wait(lock, [&transListener]() {
        return transListener->copyEvent_ == SUCCESS || transListener->copyEvent_ == FAILED;
    });
    if (transListener->copyEvent_ == FAILED) {
        return NError(EIO);
    }

    Uri uri(srcUri);
    auto authority = uri.GetAuthority();
    if (authority == FILE_MANAGER_AUTHORITY || authority == MEDIA_AUTHORITY) {
        HILOGW("Public or media path not copy");
        return NError(ERRNO_NOERR);
    }

    FileUri fileUri(destUri);
    std::string sandboxPath = fileUri.GetPath();
    if (std::filesystem::exists(sandboxPath) && std::filesystem::is_directory(sandboxPath)) {
        HILOGI("Copy dir");
        CopyDir(DISTRIBUTED_PATH, sandboxPath);
    } else {
        auto pos = srcUri.find_last_of('/');
        if (pos == std::string::npos) {
            HILOGE("invalid uri");
            return NError(EIO);
        }
        auto fileName = srcUri.substr(pos);
        auto networkIdPos = fileName.find(NETWORK_PARA);
        if (networkIdPos == std::string::npos) {
            HILOGE("Not remote uri");
            return NError(EIO);
        }
        fileName = fileName.substr(0, networkIdPos);
        std::filesystem::copy(DISTRIBUTED_PATH + fileName, sandboxPath, std::filesystem::copy_options::update_existing);
    }
    RmDirectory(DISTRIBUTED_PATH);
    return NError(ERRNO_NOERR);
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
    if (entry->progressSize <= numeric_limits<int64_t>::max() && entry->totalSize <= numeric_limits<int64_t>::max()) {
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
        delete entry;
        delete work;
        return ENOMEM;
    }
    entry->progressSize = processedBytes;
    entry->totalSize = totalBytes;
    work->data = entry;
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(CallbackComplete));
    return ERRNO_NOERR;
}

int32_t TransListener::OnFinished(const std::string &sessionName)
{
    HILOGD("OnFinished");
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    copyEvent_ = SUCCESS;
    cv_.notify_all();
    return ERRNO_NOERR;
}

int32_t TransListener::OnFailed(const std::string &sessionName)
{
    HILOGD("OnFailed");
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    copyEvent_ = FAILED;
    cv_.notify_all();
    return ERRNO_NOERR;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS