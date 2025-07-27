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
#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCH_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCH_ENTITY_H

#include <cstdint>
#include <functional>
#include <string>

#include "i_watcher_callback.h"

namespace OHOS::FileManagement::ModuleFileIO {

struct WatchEvent {
    std::string fileName = "";
    uint32_t event = 0;
    uint32_t cookie = 0;
};

struct WatcherInfo {
    std::string fileName = "";
    uint32_t events = 0;
    int32_t wd = -1;
    std::shared_ptr<IWatcherCallback> callback;

    explicit WatcherInfo(std::shared_ptr<IWatcherCallback> callback) : callback(std::move(callback)) {}

    void TriggerCallback(const std::string &eventFileName, uint32_t event, uint32_t cookie) const
    {
        callback->InvokeCallback(eventFileName, event, cookie);
    }
};

struct FsWatchEntity {
    std::shared_ptr<WatcherInfo> data_;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCH_ENTITY_H
