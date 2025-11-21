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

#include "fs_watcher_wrapper.h"

#include "ani_cache.h"
#include "ani_signature.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

FsWatcher *FsWatcherWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsWatcher err: %{public}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsWatcher *watcher = reinterpret_cast<FsWatcher *>(ptrValue);
    return watcher;
}

ani_object FsWatcherWrapper::Wrap(ani_env *env, const FsWatcher *watcher)
{
    if (watcher == nullptr) {
        HILOGE("FsWatcher pointer is null!");
        return nullptr;
    }

    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, FS::WatcherInner::classDesc);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(ret, ctor) = aniCache.GetMethod(env, FS::WatcherInner::classDesc, FS::WatcherInner::ctorDesc,
        FS::WatcherInner::ctorSig);
    if (ret != ANI_OK) {
        return nullptr;
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(watcher));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New '%{public}s' obj Failed!", FS::WatcherInner::classDesc.c_str());
        return nullptr;
    }
    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS