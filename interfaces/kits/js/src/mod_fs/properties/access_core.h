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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ACCESS_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ACCESS_CORE_H

#include "filemgmt_libfs.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

enum class AccessModeType { ERROR = -1, EXIST = 0, WRITE = 2, READ = 4, READ_WRITE = 6 };

enum AccessFlag : int32_t {
    DEFAULT_FLAG = -1,
    LOCAL_FLAG,
};

class AccessCore {
public:
    inline static const string className_ = "__properities__";

    static FsResult<bool> DoAccess(const string &path, const optional<AccessModeType> &mode = nullopt);
    static FsResult<bool> DoAccess(const string &path, const AccessModeType &mode, const AccessFlag &flag);
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ACCESS_CORE_H