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

#include "progress_listener_ani.h"

#include <memory>
#include "ani_helper.h"
#include "ani_signature.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static ani_object WrapCopyProgress(ani_env *env, uint64_t progressSize, uint64_t totalSize)
{
    auto classDesc = FS::ProgressInner::classDesc.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(classDesc, &cls)) {
        HILOGE("Cannot find class %{private}s", classDesc);
        return nullptr;
    }
    auto ctorDesc = FS::ProgressInner::ctorDesc.c_str();
    auto ctorSig = FS::ProgressInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Cannot find constructor method for class %{private}s", classDesc);
        return nullptr;
    }

    const ani_double aniProgressSize = static_cast<ani_double>(progressSize <= MAX_VALUE ? progressSize : 0);
    const ani_double aniTotalSize = static_cast<ani_double>(totalSize <= MAX_VALUE ? totalSize : 0);

    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, aniProgressSize, aniTotalSize)) {
        HILOGE("Create %{private}s object failed!", classDesc);
        return nullptr;
    }
    return obj;
}

static void SendCopyProgress(ani_vm *vm, ani_ref listener, uint64_t progressSize, uint64_t totalSize)
{
    if (vm == nullptr) {
        HILOGE("Cannot send copy progress because the vm is null.");
        return;
    }
    if (listener == nullptr) {
        HILOGE("Cannot send copy progress because the listener is null.");
        return;
    }
    ani_env *env = AniHelper::GetThreadEnv(vm);
    if (env == nullptr) {
        HILOGE("Cannot send copy progress because the env is null.");
        return;
    }
    auto evtObj = WrapCopyProgress(env, progressSize, totalSize);
    if (evtObj == nullptr) {
        HILOGE("Create copy progress obj failed!");
        return;
    }
    vector<ani_ref> args = { static_cast<ani_ref>(evtObj) };
    auto argc = args.size();
    ani_ref result;
    auto cbObj = static_cast<ani_fn_object>(listener);
    auto status = env->FunctionalObject_Call(cbObj, argc, args.data(), &result);
    if (status != ANI_OK) {
        HILOGE("Failed to call FunctionalObject_Call, status: %{public}d", static_cast<int32_t>(status));
        return;
    }
}

void ProgressListenerAni::InvokeListener(uint64_t progressSize, uint64_t totalSize) const
{
    auto localVm = vm;
    auto localListener = listener;
    auto task = [localVm, localListener, progressSize, totalSize]() {
        SendCopyProgress(localVm, localListener, progressSize, totalSize);
    };
    AniHelper::SendEventToMainThread(task);
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
