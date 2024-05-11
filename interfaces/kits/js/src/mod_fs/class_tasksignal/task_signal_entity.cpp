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

#include "task_signal_entity.h"

#include <uv.h>
namespace OHOS::FileManagement::ModuleFileIO {
TaskSignalEntity::~TaskSignalEntity() {}

void TaskSignalEntity::OnCancel(const std::string &path)
{
    uv_loop_s *loop = nullptr;
    if (!callbackContext_) {
        return;
    }
    auto env = callbackContext_->env_;
    callbackContext_->filePath_ = path;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(callbackContext_.get());
    int ret = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
                JSCallbackContext *callbackContext = reinterpret_cast<JSCallbackContext *>(work->data);
                if (callbackContext == nullptr) {
                    return;
                }
                if (!callbackContext->ref_) {
                    return;
                }
                napi_handle_scope scope = nullptr;
                napi_status ret = napi_open_handle_scope(callbackContext->env_, &scope);
                if (ret != napi_ok) {
                    return;
                }
                napi_env env = callbackContext->env_;
                napi_value jsCallback = callbackContext->ref_.Deref(env).val_;
                napi_value filePath = LibN::NVal::CreateUTF8String(env, callbackContext->filePath_).val_;
                napi_value retVal = nullptr;
                ret = napi_call_function(env, nullptr, jsCallback, 1, &filePath, &retVal);
                if (ret != napi_ok) {
                    HILOGE("Failed to call napi_call_function, ret: %{public}d", ret);
                }
                ret = napi_close_handle_scope(callbackContext->env_, scope);
                if (ret != napi_ok) {
                    HILOGE("Failed to close handle scope, ret: %{public}d", ret);
                }
                delete callbackContext;
                delete work;
            }, uv_qos_user_initiated);
    if (ret != 0) {
        HILOGE("Failed to uv_queue_work_with_qos, ret: %{public}d", ret);
        delete work;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO