/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "filemapping_wrapper.h"

#include "ani_cache.h"
#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_filemapping.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

FsFileMapping *FileMappingWrapper::Unwrap(ani_env *env, ani_object object)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return nullptr;
    }
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap file mapping err: %{public}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsFileMapping *mapping = reinterpret_cast<FsFileMapping *>(ptrValue);
    return mapping;
}

ani_object FileMappingWrapper::Wrap(ani_env *env, const FsFileMapping *mapping)
{
    if (mapping == nullptr) {
        HILOGE("FsFileMapping pointer is null!");
        return nullptr;
    }

    AniCache &aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, FS::FileMappingInner::classDesc);
    if (ret != ANI_OK) {
        HILOGE("Failed to get FileMappingInner class");
        return nullptr;
    }

    ani_method ctor;
    auto classDesc = FS::FileMappingInner::classDesc.c_str();
    tie(ret, ctor) = aniCache.GetMethod(env, FS::FileMappingInner::classDesc,
        FS::FileMappingInner::ctorDesc, FS::FileMappingInner::ctorSig);
    if (ret != ANI_OK) {
        HILOGE("Cannot find constructor method for class %{private}s", classDesc);
        return nullptr;
    }

    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(mapping));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %{public}s obj Failed!", classDesc);
        return nullptr;
    }

    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
