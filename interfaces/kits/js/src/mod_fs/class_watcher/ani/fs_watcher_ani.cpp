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

#include "fs_watcher_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_watcher_wrapper.h"
#include "fs_watcher.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

void FsWatcherAni::Start(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto watcher = FsWatcherWrapper::Unwrap(env, object);
    if (watcher == nullptr) {
        HILOGE("Cannot unwrap watcher!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = watcher->Start();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot start watcher!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void FsWatcherAni::Stop(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto watcher = FsWatcherWrapper::Unwrap(env, object);
    if (watcher == nullptr) {
        HILOGE("Cannot unwrap watcher!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = watcher->Stop();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot stop watcher!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS