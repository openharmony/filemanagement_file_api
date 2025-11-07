/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

void TaskSignalEntity::OnCancel()
{
    auto env = callbackContext_->env_;
    callbackContext_->filePath_ = taskSignal_->filePath_;
    JSCallbackContext *callbackContext = callbackContext_.get();
    auto task = [callbackContext] () {
        if (callbackContext == nullptr) {
            return;
        }
        if (!callbackContext->ref_) {
            return;
        }
        napi_handle_scope scope = nullptr;
        napi_status ret = napi_open_handle_scope(callbackContext->env_, &scope);
        if (ret != napi_ok || scope == nullptr) {
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
    };
    auto ret = napi_send_event(env, task, napi_eprio_immediate, "fs.TaskSignal.onCancel");
    if (ret != 0) {
        HILOGE("Failed to call napi_send_event, ret: %{public}d", ret);
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO