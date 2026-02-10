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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FS_FILE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FS_FILE_H

#include "file_entity.h"
#include "filemgmt_libfs.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

class FsFile {
public:
    inline static const string className_ = "File";

    FileEntity *GetFileEntity() const
    {
        return fileEntity.get();
    }

    FsFile(const FsFile &other) = delete;
    FsFile &operator=(const FsFile &other) = delete;

    FsFile(FsFile &&other) noexcept : fileEntity(move(other.fileEntity))
    {
        other.fileEntity = nullptr;
    }

    FsFile &operator=(FsFile &&other) noexcept
    {
        if (this != &other) {
            fileEntity = move(other.fileEntity);
            other.fileEntity = nullptr;
        }
        return *this;
    }

    ~FsFile() = default;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    FsResult<string> GetPath() const;
    FsResult<string> GetName() const;
    FsResult<string> GetParent() const;
    FsResult<void> Lock(bool exclusive = false) const;
    FsResult<void> TryLock(bool exclusive = false) const;
    FsResult<void> UnLock() const;
#endif
    static FsResult<FsFile *> Constructor();
    FsResult<int32_t> GetFD() const;
    void RemoveEntity();
    bool CheckFileEntityValid() const;

private:
    unique_ptr<FileEntity> fileEntity;
    explicit FsFile(unique_ptr<FileEntity> entity) : fileEntity(move(entity)) {}
};

const string PROCEDURE_LOCK_NAME = "FileIOFileLock";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FS_FILE_H