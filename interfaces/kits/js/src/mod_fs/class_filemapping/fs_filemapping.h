/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_FS_FILEMAPPING_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_FS_FILEMAPPING_H

#include <cinttypes>
#include <iostream>
#include <memory>

#include "filemgmt_libfs.h"
#include "filemgmt_libhilog.h"

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

struct ReadWriteArgs {
    size_t position;
    size_t bufferArgIndex;
    size_t lengthArgIndex;
    void *buffer;
    size_t bufLen;
    size_t opLength;
};

enum MappingMode {
    READ_ONLY = 0,
    READ_WRITE = 1,
    PRIVATE = 2
};

struct FileMappingParams {
    void *mapAddr = nullptr;
    void *rawMapAddr = nullptr;
    size_t capacity = 0;
    size_t rawCapacity = 0;
    size_t adjustment = 0;
    int mode = MappingMode::READ_ONLY;
    int fd = -1;
    off_t offset = 0;
};

class FsFileMapping;

struct FileMappingEntity {
    FsFileMapping *fsMapping = nullptr;

    void *mapAddr = nullptr;
    void *rawMapAddr = nullptr;
    size_t capacity = 0;
    size_t rawCapacity = 0;
    size_t adjustment = 0;
    size_t limit = 0;
    size_t position = 0;
    int mode = MappingMode::READ_ONLY;
    int fd = -1;
    off_t offset = 0;
    bool isValid = false;

    FileMappingEntity() = default;
    ~FileMappingEntity();

    bool CheckValid() const
    {
        return isValid && mapAddr != nullptr;
    }

    bool IsReadOnly() const
    {
        return mode == MappingMode::READ_ONLY;
    }
};

class FsFileMapping {
public:
    inline static const std::string className_ = "FileMapping";

    FileMappingEntity *GetEntity() const
    {
        return entity_.get();
    }

    FsFileMapping(const FsFileMapping &other) = delete;
    FsFileMapping &operator=(const FsFileMapping &other) = delete;

    FsFileMapping(FsFileMapping &&other) noexcept : entity_(std::move(other.entity_))
    {
        other.entity_ = nullptr;
    }

    FsFileMapping &operator=(FsFileMapping &&other) noexcept
    {
        if (this != &other) {
            entity_ = std::move(other.entity_);
            other.entity_ = nullptr;
        }
        return *this;
    }

    ~FsFileMapping() = default;

    static FsResult<FsFileMapping *> Constructor(const FileMappingParams &params);
    FsResult<void> SetPosition(size_t position);
    FsResult<size_t> GetPosition() const;
    FsResult<size_t> Capacity() const;
    FsResult<void> SetLimit(size_t limit);
    FsResult<size_t> GetLimit() const;
    FsResult<void> Flip();
    FsResult<size_t> Remaining() const;
    FsResult<size_t> Read(void *buffer, size_t bufLen, size_t length);
    FsResult<size_t> ReadFrom(size_t position, void *buffer, size_t bufLen, size_t length);
    FsResult<size_t> Write(const void *data, size_t dataLen, size_t length);
    FsResult<size_t> WriteTo(size_t position, const void *data, size_t dataLen, size_t length);
    FsResult<void> Msync(size_t offset, size_t length);
    FsResult<void> Unmap();
    bool CheckValid() const;
    bool IsReadOnly() const;

private:
    std::unique_ptr<FileMappingEntity> entity_;
    explicit FsFileMapping(std::unique_ptr<FileMappingEntity> entity) : entity_(std::move(entity)) {}
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_FS_FILEMAPPING_H
