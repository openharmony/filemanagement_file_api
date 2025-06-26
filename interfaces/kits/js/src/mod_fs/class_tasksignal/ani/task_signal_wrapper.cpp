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

#include "task_signal_wrapper.h"

#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_task_signal.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

FsTaskSignal *TaskSignalWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto status = env->Object_GetFieldByName_Long(object, "nativeTaskSignal", &nativePtr);
    if (status != ANI_OK) {
        HILOGE("Unwrap taskSignal obj failed! status: %{public}d", status);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsTaskSignal *copySignal = reinterpret_cast<FsTaskSignal *>(ptrValue);
    return copySignal;
}

bool TaskSignalWrapper::Wrap(ani_env *env, ani_object object, const FsTaskSignal *signal)
{
    if (object == nullptr) {
        HILOGE("TaskSignal obj is null!");
        return false;
    }

    if (signal == nullptr) {
        HILOGE("FsTaskSignal pointer is null!");
        return false;
    }

    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(signal));

    auto status = env->Object_SetFieldByName_Long(object, "nativeTaskSignal", ptr);
    if (status != ANI_OK) {
        HILOGE("Wrap taskSignal obj failed! status: %{public}d", status);
        return false;
    }

    return true;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS