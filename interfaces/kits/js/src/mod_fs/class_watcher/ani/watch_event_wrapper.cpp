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

ani_object WatchEventWrapper::Wrap(ani_env *env, const WatchEvent &evt)
{
    auto classDesc = FS::WatchEventInner::classDesc.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(classDesc, &cls)) {
        HILOGE("Cannot find class %{public}s", classDesc);
        return nullptr;
    }
    auto ctorDesc = FS::WatchEventInner::ctorDesc.c_str();
    auto ctorSig = FS::WatchEventInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Cannot find constructor method for class %{public}s", classDesc);
        return nullptr;
    }

    auto [succ, fileName] = TypeConverter::ToAniString(env, evt.fileName);
    if (!succ) {
        HILOGE("Convert fileName to ani string failed!");
        return nullptr;
    }

    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, fileName, evt.event, evt.cookie)) {
        HILOGE("Create %{public}s obj failed!", classDesc);
        return nullptr;
    }
    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS