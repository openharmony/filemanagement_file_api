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

#include "stream_wrapper.h"

#include <string>

#include "ani_helper.h"
#include "ani_signature.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

FsStream *StreamWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsFile err: %{private}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsStream *stream = reinterpret_cast<FsStream *>(ptrValue);
    return stream;
}

ani_object StreamWrapper::Wrap(ani_env *env, const FsStream *stream)
{
    if (stream == nullptr) {
        HILOGE("FsStream pointer is null!");
        return nullptr;
    }
    auto classDesc = FS::StreamInner::classDesc.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(classDesc, &cls)) {
        HILOGE("Cannot find class %s", classDesc);
        return nullptr;
    }
    auto ctorDesc = FS::StreamInner::ctorDesc.c_str();
    auto ctorSig = FS::StreamInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Cannot find constructor method for class %s", classDesc);
        return nullptr;
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(stream));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", classDesc);
        return nullptr;
    }

    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS