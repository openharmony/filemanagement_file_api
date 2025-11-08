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

#include "ani_cache.h"
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
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, FS::WatchEventInner::classDesc);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(ret, ctor) = aniCache.GetMethod(env, FS::WatchEventInner::classDesc, FS::WatchEventInner::ctorDesc,
        FS::WatchEventInner::ctorSig);
    if (ret != ANI_OK) {
        return nullptr;
    }

    auto [succ, fileName] = TypeConverter::ToAniString(env, evt.fileName);
    if (!succ) {
        HILOGE("Convert fileName to ani string failed!");
        return nullptr;
    }

    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, fileName, evt.event, evt.cookie)) {
        HILOGE("Create %{public}s obj failed!", FS::WatchEventInner::classDesc.c_str());
        return nullptr;
    }
    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS