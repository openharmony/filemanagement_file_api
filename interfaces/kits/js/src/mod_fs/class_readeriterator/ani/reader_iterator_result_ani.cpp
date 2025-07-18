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

#include "reader_iterator_result_ani.h"

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

ani_object ReaderIteratorResultAni::Wrap(ani_env *env, const ReaderIteratorResult &result)
{
    auto classDesc = FS::ReaderIteratorResultInner::classDesc.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(classDesc, &cls)) {
        HILOGE("Cannot find class %s", classDesc);
        return nullptr;
    }

    auto ctorDesc = FS::ReaderIteratorResultInner::ctorDesc.c_str();
    auto ctorSig = FS::ReaderIteratorResultInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Cannot find constructor method for class %s", classDesc);
        return nullptr;
    }

    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj)) {
        HILOGE("New %s obj Failed!", classDesc);
        return nullptr;
    }

    if (ANI_OK != AniHelper::SetPropertyValue(env, obj, "done", result.done)) {
        HILOGE("Set 'done' field value failed!");
        return nullptr;
    }

    if (ANI_OK != AniHelper::SetPropertyValue(env, obj, "value", result.value)) {
        HILOGE("Set 'value' field value failed!");
        return nullptr;
    }

    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS