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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_DIR_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_DIR_CORE_H

#include <optional>
#include <string>

#include "filemgmt_libfs.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

constexpr int COPYMODE_MIN = 0;
constexpr int COPYMODE_MAX = 1;
constexpr int COPYDIR_DEFAULT_PERM = 0770;

constexpr int DISMATCH = 0;
constexpr int MATCH = 1;

enum ModeOfCopyDir { DIRMODE_FILE_COPY_THROW_ERR = 0, DIRMODE_FILE_COPY_REPLACE };

struct CopyDirResult {
    FsResult<void> fsResult;
    optional<vector<struct ConflictFiles>> errFiles;
};

class CopyDirCore final {
public:
    static struct CopyDirResult DoCopyDir(
        const string &src, const string &dest, const optional<int32_t> &mode = nullopt);
};

struct ConflictFiles {
    string srcFiles;
    string destFiles;
    ConflictFiles(const string &src, const string &dest) : srcFiles(src), destFiles(dest) {}
    ~ConflictFiles() = default;
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_DIR_CORE_H