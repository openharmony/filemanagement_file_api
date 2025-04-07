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

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

ani_object ReaderIteratorResultAni::Wrap(ani_env *env, const ReaderIteratorResult *result)
{
    static const char *className = "L@ohos/file/fs/fileIo/ReaderIteratorResultInner;";

    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(result));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", className);
        return nullptr;
    }

    const auto &done = result->done;
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "done", static_cast<bool>(done))) {
        HILOGE("Set 'done' field value failed!");
        return nullptr;
    }

    const auto &value = result->value;
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "value", value)) {
        HILOGE("Set 'value' field value failed!");
        return nullptr;
    }

    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS