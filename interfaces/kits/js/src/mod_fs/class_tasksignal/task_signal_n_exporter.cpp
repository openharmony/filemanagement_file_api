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

#include "task_signal_n_exporter.h"

#include "file_utils.h"
#include "task_signal.h"
#include "task_signal_entity.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace DistributedFS::ModuleTaskSignal;
constexpr int NO_ERROR = 0;
constexpr int CANCEL_ERR = -3;
TaskSignalNExporter::TaskSignalNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

TaskSignalNExporter::~TaskSignalNExporter() {}

string TaskSignalNExporter::GetClassName()
{
    return TaskSignalNExporter::className_;
}

napi_value TaskSignalNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Failed to get param.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto taskSignalEntity = CreateUniquePtr<TaskSignalEntity>();
    if (taskSignalEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    taskSignalEntity->taskSignal_ = std::make_shared<TaskSignal>();
    if (!NClass::SetEntityFor<TaskSignalEntity>(env, funcArg.GetThisVar(), std::move(taskSignalEntity))) {
        HILOGE("Failed to set watcherEntity.");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool TaskSignalNExporter::Export()
{
    vector<napi_property_descriptor> props = {
            NVal::DeclareNapiFunction("cancel", Cancel),
            NVal::DeclareNapiFunction("onCancel", OnCancel),
    };
    string className = GetClassName();

    auto [succ, classValue] = NClass::DefineClass(exports_.env_, className_, Constructor, std::move(props));
    if (!succ) {
        HILOGE("Failed to DefineClass");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Failed to SaveClass");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    return exports_.AddProp(className, classValue);
}

napi_value TaskSignalNExporter::Cancel(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Failed to get param when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto taskSignalEntity = NClass::GetEntityOf<TaskSignalEntity>(env, funcArg.GetThisVar());
    if (!taskSignalEntity || taskSignalEntity->taskSignal_ == nullptr) {
        HILOGE("Failed to get watcherEntity when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto ret = taskSignalEntity->taskSignal_->Cancel();
    if (ret != NO_ERROR) {
        HILOGE("Failed to cancel the task.");
        NError(CANCEL_ERR).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value TaskSignalNExporter::OnCancel(napi_env env, napi_callback_info info)
{
    HILOGD("TaskSignal OnCancel Func Run in.");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Failed to get param when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto taskSignalEntity = NClass::GetEntityOf<TaskSignalEntity>(env, funcArg.GetThisVar());
    if (!taskSignalEntity || taskSignalEntity->taskSignal_ == nullptr) {
        HILOGE("Failed to get watcherEntity when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    std::shared_ptr<TaskSignalEntity> signal(taskSignalEntity);
    taskSignalEntity->taskSignal_->SetTaskSignalListener(signal);
    auto callbackContext = std::make_shared<JSCallbackContext>(NVal(env, funcArg[0]));
    callbackContext->env_ = env;
    taskSignalEntity->callbackContext_ = callbackContext;
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}
} // namespace OHOS::FileManagement::ModuleFileIO