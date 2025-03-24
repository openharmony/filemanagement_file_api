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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_I_WATCHER_CALLBACK_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_I_WATCHER_CALLBACK_H

#include <string>

namespace OHOS::FileManagement::ModuleFileIO {

class IWatcherCallback {
public:
    virtual ~IWatcherCallback() = default;
    virtual bool IsStrictEquals(const std::shared_ptr<IWatcherCallback> &other) const = 0;
    virtual void InvokeCallback(const std::string &fileName, uint32_t event, uint32_t cookie) const = 0;
    virtual std::string GetClassName() const = 0;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_I_WATCHER_CALLBACK_H