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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_LISTENER_I_PROGRESS_LISTENER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_LISTENER_I_PROGRESS_LISTENER_H

#include <string>

namespace OHOS::FileManagement::ModuleFileIO {
const uint64_t MAX_VALUE = 0x7FFFFFFFFFFFFFFF;

class IProgressListener {
public:
    virtual ~IProgressListener() = default;
    virtual void InvokeListener(uint64_t progressSize, uint64_t totalSize) const = 0;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_LISTENER_I_PROGRESS_LISTENER_H