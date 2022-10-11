/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_STAT_V9_STAT_ENTITYV9_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_STAT_V9_STAT_ENTITYV9_H

#include <sys/stat.h>

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
struct StatEntityV9 {
    struct stat stat_;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif