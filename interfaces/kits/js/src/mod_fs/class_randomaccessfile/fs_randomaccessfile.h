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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_FS_RANDOMACCESSFILE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_FS_RANDOMACCESSFILE_H

#include "filemgmt_libfs.h"
#include "randomaccessfile_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

struct WriteOptions {
    optional<size_t> length = nullopt;
    optional<int64_t> offset = nullopt;
    optional<string> encoding = nullopt;
};

struct ReadOptions {
    optional<int64_t> offset = nullopt;
    optional<int64_t> length = nullopt;
};

class FsRandomAccessFile {
public:
    inline static const string className_ = "RandomAccessFile";

    RandomAccessFileEntity *GetRAFEntity() const
    {
        return rafEntity.get();
    }

    FsRandomAccessFile(const FsRandomAccessFile &other) = delete;
    FsRandomAccessFile &operator=(const FsRandomAccessFile &other) = delete;

    FsRandomAccessFile(FsRandomAccessFile &&other) noexcept : rafEntity(move(other.rafEntity))
    {
        other.rafEntity = nullptr;
    }

    FsRandomAccessFile &operator=(FsRandomAccessFile &&other) noexcept
    {
        if (this != &other) {
            rafEntity = move(other.rafEntity);
            other.rafEntity = nullptr;
        }
        return *this;
    }

    ~FsRandomAccessFile() = default;

    FsResult<void> SetFilePointerSync(const int64_t &fp) const;
    FsResult<int64_t> WriteSync(const string &buffer, const optional<WriteOptions> &options = nullopt) const;
    FsResult<int64_t> WriteSync(const ArrayBuffer &buffer, const optional<WriteOptions> &options = nullopt) const;
    FsResult<int64_t> ReadSync(ArrayBuffer &buffer, const optional<ReadOptions> &options = nullopt) const;
    FsResult<void> CloseSync() const;
    FsResult<int32_t> GetFD() const;
    FsResult<int64_t> GetFPointer() const;

    static FsResult<FsRandomAccessFile *> Constructor();

private:
    unique_ptr<RandomAccessFileEntity> rafEntity;
    explicit FsRandomAccessFile(unique_ptr<RandomAccessFileEntity> entity) : rafEntity(move(entity)) {}
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif //INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_FS_RANDOMACCESSFILE_H