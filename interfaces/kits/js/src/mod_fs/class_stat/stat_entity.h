/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_STAT_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_STAT_ENTITY_H

#include <memory>

#include "common_func.h"
#include "uv.h"
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
enum Location {
    LOCAL = 1 << 0,
    CLOUD = 1 << 1
};
#endif
namespace OHOS::FileManagement::ModuleFileIO {
struct StatEntity {
    uv_stat_t stat_;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    std::shared_ptr<FileInfo> fileInfo_;
#endif
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_STAT_ENTITY_H