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

#include "watcher_ani.h"

#include "ani_helper.h"
#include "error_handler.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_watcher_wrapper.h"
#include "type_converter.h"
#include "watch_event_listener.h"
#include "watcher_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;

ani_object WatcherAni::CreateWatcherSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_int events, ani_ref listener)
{
    auto [succPath, filePath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    ani_ref cbRef;
    if (ANI_OK != env->GlobalReference_Create(move(listener), &cbRef)) {
        HILOGE("Failed to get callback");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    ani_vm *vm = nullptr;
    env->GetVM(&vm);
    auto eventListener = CreateSharedPtr<WatchEventListener>(vm, cbRef);
    if (eventListener == nullptr) {
        HILOGE("Failed to request heap memory.");
        ErrorHandler::Throw(env, ENOMEM);
        return nullptr;
    }

    FsResult<FsWatcher *> ret =
        WatcherCore::DoCreateWatcher(filePath, events, move(eventListener));
    if (!ret.IsSuccess()) {
        HILOGE("Create watcher failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    const FsWatcher *watcher = ret.GetData().value();
    auto result = FsWatcherWrapper::Wrap(env, move(watcher));
    if (result == nullptr) {
        HILOGE("Failed to Wrap");
        delete watcher;
        watcher = nullptr;
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS