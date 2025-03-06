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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_WRITE_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_WRITE_CORE_H

#include "filemgmt_libfs.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

struct WriteOptions {
    optional<size_t> length = nullopt;
    optional<int64_t> offset = nullopt;
    optional<string> encoding = nullopt;
};

class WriteCore final {
public:
    static FsResult<int64_t> DoWrite(
        const int32_t fd, const string &buffer, const optional<WriteOptions> &options = nullopt);
    static FsResult<int64_t> DoWrite(
        const int32_t fd, const ArrayBuffer &buffer, const optional<WriteOptions> &options = nullopt);

private:
    static FsResult<int64_t> DoWrite(const int32_t fd, void *buf, const size_t len, const int64_t offset);
};

const string PROCEDURE_WRITE_NAME = "FileIOWrite";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_WRITE_CORE_H