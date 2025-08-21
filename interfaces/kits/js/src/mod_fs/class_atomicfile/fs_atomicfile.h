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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_ATOMICFILE_FS_ATOMICFILE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_ATOMICFILE_FS_ATOMICFILE_H

#include "fs_atomicfile_entity.h"
#include "fs_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

struct BufferData {
    uint8_t *buffer = nullptr;
    size_t length = 0;

    ~BufferData()
    {
        delete[] buffer;
    }
};

class FsAtomicFile final {
public:
    FsAtomicFileEntity *GetEntity();
    static FsResult<FsAtomicFile *> Constructor(string path);
    string GetPath();
    FsResult<FsFile *> GetBaseFile();
    FsResult<unique_ptr<BufferData>> ReadFully();
    FsResult<string> StartWrite();
    FsResult<void> FinishWrite();
    FsResult<void> FailWrite();
    FsResult<void> Delete();
    static void FinalizeCallback(void *finalizeData, [[maybe_unused]] void *finalizeHint);
    ~FsAtomicFile() {
        HILOGE("~FsAtomicFile success.");
    }

private:
    unique_ptr<FsAtomicFileEntity> entity;
    explicit FsAtomicFile(unique_ptr<FsAtomicFileEntity> entity) : entity(move(entity)) {}
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_ATOMICFILE_FS_ATOMICFILE_H