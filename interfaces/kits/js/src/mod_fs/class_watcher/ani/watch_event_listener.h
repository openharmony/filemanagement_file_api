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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_ANI_WATCH_EVENT_LISTENER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_ANI_WATCH_EVENT_LISTENER_H

#include <string>

#include <ani.h>

#include "fs_watch_entity.h"
#include "i_watcher_callback.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {

class WatchEventListener final : public IWatcherCallback {
public:
    WatchEventListener(ani_vm *vm, const ani_ref &callback) : vm(vm), callback(callback) {}
    bool IsStrictEquals(const std::shared_ptr<IWatcherCallback> &other) const override;
    void InvokeCallback(const std::string &fileName, uint32_t event, uint32_t cookie) const override;

    std::string GetClassName() const override
    {
        return className_;
    }

private:
    inline static const std::string className_ = "WatchEventListener";
    ani_vm *vm;
    ani_ref callback;
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_ANI_WATCH_EVENT_LISTENER_H