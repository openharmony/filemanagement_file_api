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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_CORE_H

#include <deque>
#include <string>

#include "filemgmt_libfs.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

constexpr int DIRMODE_MIN = 0;
constexpr int DIRMODE_MAX = 3;

constexpr int FILE_DISMATCH = 0;
constexpr int FILE_MATCH = 1;
constexpr int MOVEDIR_DEFAULT_PERM = 0770;

enum ModeOfMoveDir {
    DIRMODE_DIRECTORY_THROW_ERR = 0,
    DIRMODE_FILE_THROW_ERR,
    DIRMODE_FILE_REPLACE,
    DIRMODE_DIRECTORY_REPLACE
};

struct ErrFiles {
    std::string srcFiles;
    std::string destFiles;
    ErrFiles(const std::string& src, const std::string& dest) : srcFiles(src), destFiles(dest) {}
    ~ErrFiles() = default;
};

struct MoveDirResult {
    FsResult<void> fsResult;
    optional<deque<struct ErrFiles>> errFiles;
};

class MoveDirCore final {
public:
    static MoveDirResult DoMoveDir(
        const std::string &src, const std::string &dest, std::optional<int32_t> mode = std::nullopt);
};

const std::string PROCEDURE_MOVEDIR_NAME = "FileIOMoveDir";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_CORE_H