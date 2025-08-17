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

#include "task_signal_listener_ani.h"

#include <memory>
#include "ani_helper.h"
#include "ani_signature.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

bool TaskSignalListenerAni::CreateGlobalReference()
{
    if (signalRef) {
        return true;
    }
    ani_env *env = AniHelper::GetThreadEnv(vm);
    int ret = 0;
    if ((ret = env->GlobalReference_Create(static_cast<ani_ref>(signalObj), &signalRef)) != ANI_OK) {
        HILOGE("TaskSignalListenerAni GlobalReference_Create failed: %{public}d", ret);
        signalRef = nullptr;
        return false;
    }
    return true;
}

void TaskSignalListenerAni::OnCancel()
{
    auto filepath = taskSignal->filePath_;
    auto task = [this, filepath]() { SendCancelEvent(filepath); };
    AniHelper::SendEventToMainThread(task);
}

void TaskSignalListenerAni::SendCancelEvent(const string &filepath) const
{
    if (vm == nullptr) {
        HILOGE("Cannot send cancel event because the vm is null.");
        return;
    }
    if (signalRef == nullptr) {
        HILOGE("Cannot send cancel event because the signalRef is null.");
        return;
    }

    ani_env *env = AniHelper::GetThreadEnv(vm);
    if (env == nullptr) {
        HILOGE("Cannot send cancel event because the env is null.");
        return;
    }
    auto [succ, aniPath] = TypeConverter::ToAniString(env, filepath);
    if (!succ) {
        HILOGE("Cannot convert filepath to ani string!");
        return;
    }

    auto ret = env->Object_CallMethodByName_Void(static_cast<ani_object>(signalRef), "onCancelCallback", nullptr,
        aniPath);
    if (ret != ANI_OK) {
        HILOGE("Call onCancelCallback failed, err: %{public}d", ret);
        return;
    }
}

TaskSignalListenerAni::~TaskSignalListenerAni()
{
    if (signalRef == nullptr) {
        return;
    }
    ani_env *env = AniHelper::GetThreadEnv(vm);
    int ret = 0;
    if ((ret = env->GlobalReference_Delete(signalRef)) != ANI_OK) {
        HILOGE("TaskSignalListenerAni GlobalReference_Delete: %{public}d", ret);
    }
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
