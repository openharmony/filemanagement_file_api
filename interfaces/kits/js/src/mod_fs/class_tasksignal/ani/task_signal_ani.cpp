/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "task_signal_ani.h"

#include "ani_helper.h"
#include "copy_core.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "task_signal_entity_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

static TaskSignalEntityCore *Unwrap(ani_env *env, ani_object object)
{
    ani_long entity;
    auto ret = env->Object_GetFieldByName_Long(object, "nativeTaskSignal", &entity);
    if (ret != ANI_OK) {
        HILOGE("Unwrap taskSignalEntityCore err: %{private}d", ret);
        return nullptr;
    }
    return reinterpret_cast<TaskSignalEntityCore *>(entity);
}

void TaskSignalAni::Cancel(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto entity = Unwrap(env, object);
    if (entity == nullptr) {
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    if (entity->taskSignal_ == nullptr) {
        HILOGE("Failed to get watcherEntity when stop.");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = entity->taskSignal_->Cancel();
    if (ret != NO_ERROR) {
        HILOGE("Failed to cancel the task.");
        ErrorHandler::Throw(env, CANCEL_ERR);
        return;
    }
}

void TaskSignalAni::OnCancel(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto entity = Unwrap(env, object);
    if (entity == nullptr) {
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    if (entity->taskSignal_ == nullptr) {
        HILOGE("Failed to get watcherEntity when stop.");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    ani_ref globalObj;
    auto status = env->GlobalReference_Create(object, &globalObj);
    if (status != ANI_OK) {
        HILOGE("GlobalReference_Create, err: %{private}d", status);
        return;
    }
    ani_vm *vm = nullptr;
    env->GetVM(&vm);
    auto cb = [vm, &globalObj](string filePath) -> void {
        auto env = AniHelper::GetThreadEnv(vm);
        if (env == nullptr) {
            HILOGE("failed to GetThreadEnv");
            return;
        }

        // std::vector<ani_ref> vec;
        auto [succPath, path] = TypeConverter::ToAniString(env, filePath);
        if (!succPath) {
            HILOGE("ToAniString failed");
            return;
        }

        auto ret = env->Object_CallMethodByName_Void(static_cast<ani_object>(globalObj),
                                                     "onCancelCallback", nullptr, path);
        if (ret != ANI_OK) {
            HILOGE("Call onCancelCallback failed, err: %{private}d", ret);
            return;
        }
    };
    auto callbackContext = std::make_shared<CallbackContextCore>(cb);
    entity->callbackContextCore_ = callbackContext;
    entity->taskSignal_->SetTaskSignalListener(entity);
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS