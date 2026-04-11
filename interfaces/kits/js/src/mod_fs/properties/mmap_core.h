/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MMAP_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MMAP_CORE_H

#include <tuple>

#include "filemgmt_libfs.h"
#include "fs_filemapping.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

struct MmapArgs {
    int fd = -1;
    int mode = 0;
    off_t offset = 0;
    size_t size = 0;
    void *mapAddr = nullptr;
};

class MmapCore {
public:
    static FsResult<FsFileMapping *> DoMmap(int fd, int mode, off_t offset, size_t size);

private:
    static std::tuple<int, int> GetMmapProtFlags(int mode);
    static int ValidateFile(int fd);
    static int ExpandFileIfNeeded(int fd, int mode, off_t offset, size_t size);
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MMAP_CORE_H
