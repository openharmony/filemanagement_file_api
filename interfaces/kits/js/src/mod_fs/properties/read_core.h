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

#ifndef FILEMANAGEMENT_FILE_API_READ_CORE_H
#define FILEMANAGEMENT_FILE_API_READ_CORE_H

#include <cstdint>
#include "filemgmt_libfs.h"
#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

struct ReadOptions final {
    optional<int64_t> offset = nullopt;
    optional<int64_t> length = nullopt;
};

class ReadCore final {
public:
    static FsResult<int64_t> DoRead(
        const int32_t &fd, ArrayBuffer &arrayBuffer, const optional<ReadOptions> &options = nullopt);
};

const string PROCEDURE_READTEXT_NAME = "FileIOReadText";
} // namespace OHOS::FileManagement::ModuleFileIO

#endif // FILEMANAGEMENT_FILE_API_READ_CORE_H