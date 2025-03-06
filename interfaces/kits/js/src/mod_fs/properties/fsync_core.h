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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FSYNC_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FSYNC_CORE_H

#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::ModuleFileIO {

class FsyncCore final {
public:
    static FsResult<void> DoFsync(const int32_t &fd);
};
const std::string PROCEDURE_FSYNC_NAME = "FileIOFsync";
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FSYNC_CORE_H