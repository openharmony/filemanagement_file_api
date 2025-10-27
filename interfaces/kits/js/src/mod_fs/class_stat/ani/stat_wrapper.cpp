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

#include "stat_wrapper.h"

#include <iostream>
#include <string>
#include <string_view>

#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

ani_object StatWrapper::Wrap(ani_env *env, FsStat *fsStat)
{
    if (fsStat == nullptr) {
        HILOGE("FsStat pointer is null!");
        return nullptr;
    }
    auto classDesc = FS::StatInner::classDesc.c_str();
    ani_object statObject = {};
    ani_class cls;
    ani_status ret;

    if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
        HILOGE("Not found %{public}s, err: %{public}d", classDesc, ret);
        return nullptr;
    }

    auto ctorDesc = FS::StatInner::ctorDesc.c_str();
    auto ctorSig = FS::StatInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Not found ctor, err: %{public}d", ret);
        return nullptr;
    }

    if ((ret = env->Object_New(cls, ctor, &statObject, reinterpret_cast<ani_long>(fsStat))) != ANI_OK) {
        HILOGE("New StatInner Fail, err: %{public}d", ret);
        return nullptr;
    }

    return statObject;
}

FsStat *StatWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long fsStat;
    auto ret = env->Object_GetFieldByName_Long(object, "nativeStat", &fsStat);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsStat err: %{public}d", ret);
        return nullptr;
    }
    return reinterpret_cast<FsStat *>(fsStat);
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS