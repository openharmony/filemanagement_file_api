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

#include "distributed_file_daemon_manager.h"
#include "ipc_skeleton.h"
#include "uri.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
const std::string NETWORK_PARA = "?networkid=";

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
    auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().PrepareSession(
        srcUri, destUri, networkId, transListener);
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