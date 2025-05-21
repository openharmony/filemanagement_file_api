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

#include "watch_event_wrapper.h"

#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

ani_object WatchEventWrapper::Wrap(ani_env *env, const WatchEvent &evt)
{
    static const char *className = "L@ohos/file/fs/WatchEventInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;DD:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }

    auto [succ, fileName] = TypeConverter::ToAniString(env, evt.fileName);
    if (!succ) {
        HILOGE("Convert fileName to ani string failed!");
        return nullptr;
    }

    auto event = static_cast<double>(evt.event);
    auto cookie = static_cast<double>(evt.cookie);

    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, fileName, event, cookie)) {
        HILOGE("Create %s obj failed!", className);
        return nullptr;
    }
    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS