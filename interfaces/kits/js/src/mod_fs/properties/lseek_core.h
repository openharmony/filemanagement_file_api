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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LSEEK_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LSEEK_CORE_H

#include "filemgmt_libfs.h"
#include "rust_file.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

class LseekCore final {
public:
    static FsResult<int64_t> DoLseek(const int32_t &fd, const int64_t &offset,
                                     const optional<SeekPos> &pos = nullopt);
};

} // namespace OHOS::FileManagement::ModuleFileIO

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LSEEK_CORE_H