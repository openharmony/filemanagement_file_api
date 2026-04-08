/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fs_filemapping.h"

#include <securec.h>

#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FileMappingEntity::~FileMappingEntity()
{
    if (fsMapping) {
        delete fsMapping;
        fsMapping = nullptr;
        mapAddr = nullptr;
        rawMapAddr = nullptr;
        isValid = false;
        return;
    }
    if (isValid && rawMapAddr != nullptr) {
        munmap(rawMapAddr, rawCapacity);
        rawMapAddr = nullptr;
        mapAddr = nullptr;
        isValid = false;
    }
}

bool FsFileMapping::CheckValid() const
{
    if (!entity_) {
        HILOGE("Failed to get file mapping entity");
        return false;
    }
    return entity_->CheckValid();
}

bool FsFileMapping::IsReadOnly() const
{
    if (!entity_) {
        return true;
    }
    return entity_->IsReadOnly();
}

FsResult<FsFileMapping *> FsFileMapping::Constructor(const FileMappingParams &params)
{
    auto entity = CreateUniquePtr<FileMappingEntity>();
    if (entity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsFileMapping *>::Error(ENOMEM);
    }

    entity->mapAddr = params.mapAddr;
    entity->rawMapAddr = params.rawMapAddr;
    entity->capacity = params.capacity;
    entity->rawCapacity = params.rawCapacity;
    entity->adjustment = params.adjustment;
    entity->limit = params.capacity;
    entity->position = 0;
    entity->mode = params.mode;
    entity->fd = params.fd;
    entity->offset = params.offset;
    entity->isValid = true;

    FsFileMapping *mapping = new (nothrow) FsFileMapping(move(entity));
    if (mapping == nullptr) {
        HILOGE("Failed to create FsFileMapping object on heap.");
        return FsResult<FsFileMapping *>::Error(ENOMEM);
    }

    return FsResult<FsFileMapping *>::Success(mapping);
}

FsResult<void> FsFileMapping::SetPosition(size_t position)
{
    if (!CheckValid()) {
        return FsResult<void>::Error(EINVAL);
    }

    if (position > entity_->limit) {
        HILOGE("Invalid position value");
        return FsResult<void>::Error(EINVAL);
    }

    entity_->position = position;
    return FsResult<void>::Success();
}

FsResult<size_t> FsFileMapping::GetPosition() const
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    return FsResult<size_t>::Success(entity_->position);
}

FsResult<size_t> FsFileMapping::Capacity() const
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    return FsResult<size_t>::Success(entity_->capacity);
}

FsResult<void> FsFileMapping::SetLimit(size_t limit)
{
    if (!CheckValid()) {
        return FsResult<void>::Error(EINVAL);
    }

    if (limit > entity_->capacity) {
        HILOGE("Invalid limit value");
        return FsResult<void>::Error(EINVAL);
    }

    entity_->limit = limit;
    if (entity_->position > entity_->limit) {
        entity_->position = entity_->limit;
    }
    return FsResult<void>::Success();
}

FsResult<size_t> FsFileMapping::GetLimit() const
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    return FsResult<size_t>::Success(entity_->limit);
}

FsResult<void> FsFileMapping::Flip()
{
    if (!CheckValid()) {
        return FsResult<void>::Error(EINVAL);
    }

    entity_->limit = entity_->position;
    entity_->position = 0;
    return FsResult<void>::Success();
}

FsResult<size_t> FsFileMapping::Remaining() const
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    size_t remaining = (entity_->limit > entity_->position) ? (entity_->limit - entity_->position) : 0;
    return FsResult<size_t>::Success(remaining);
}

FsResult<size_t> FsFileMapping::Read(void *buffer, size_t bufLen, size_t length)
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    if (buffer == nullptr && bufLen > 0) {
        HILOGE("Invalid buffer argument");
        return FsResult<size_t>::Error(EINVAL);
    }

    size_t remaining = (entity_->position < entity_->limit) ? (entity_->limit - entity_->position) : 0;
    size_t opLength = (length < remaining) ? length : remaining;
    if (opLength > bufLen) {
        opLength = bufLen;
    }

    if (opLength > 0) {
        int ret = memcpy_s(buffer, bufLen, static_cast<char *>(entity_->mapAddr) + entity_->position, opLength);
        if (ret != 0) {
            HILOGE("Memcpy_s failed, error: %{public}d", ret);
            return FsResult<size_t>::Error(EIO);
        }
        entity_->position += opLength;
    }

    return FsResult<size_t>::Success(opLength);
}

FsResult<size_t> FsFileMapping::ReadFrom(size_t position, void *buffer, size_t bufLen, size_t length)
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    if ((buffer == nullptr && bufLen > 0) || position > entity_->capacity) {
        HILOGE("Invalid argument");
        return FsResult<size_t>::Error(EINVAL);
    }

    size_t remaining = (position < entity_->limit) ? (entity_->limit - position) : 0;
    size_t opLength = (length < remaining) ? length : remaining;
    if (opLength > bufLen) {
        opLength = bufLen;
    }

    if (opLength > 0) {
        int ret = memcpy_s(buffer, bufLen, static_cast<char *>(entity_->mapAddr) + position, opLength);
        if (ret != 0) {
            HILOGE("Memcpy_s failed, error: %{public}d", ret);
            return FsResult<size_t>::Error(EIO);
        }
    }

    return FsResult<size_t>::Success(opLength);
}

FsResult<size_t> FsFileMapping::Write(const void *data, size_t dataLen, size_t length)
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    if (data == nullptr && dataLen > 0) {
        HILOGE("Invalid data argument");
        return FsResult<size_t>::Error(EINVAL);
    }

    if (entity_->IsReadOnly()) {
        HILOGE("Read-only mmap buffer");
        return FsResult<size_t>::Error(FILEIO_SYS_CAP_TAG + E_MMAP_RO);
    }

    size_t remaining = (entity_->position < entity_->limit) ? (entity_->limit - entity_->position) : 0;
    size_t opLength = (length < remaining) ? length : remaining;
    if (opLength > dataLen) {
        opLength = dataLen;
    }

    if (opLength > 0) {
        int ret = memcpy_s(static_cast<char *>(entity_->mapAddr) + entity_->position,
            entity_->capacity - entity_->position, data, opLength);
        if (ret != 0) {
            HILOGE("Memcpy_s failed, error: %{public}d", ret);
            return FsResult<size_t>::Error(EIO);
        }
        entity_->position += opLength;
    }

    return FsResult<size_t>::Success(opLength);
}

FsResult<size_t> FsFileMapping::WriteTo(size_t position, const void *data, size_t dataLen, size_t length)
{
    if (!CheckValid()) {
        return FsResult<size_t>::Error(EINVAL);
    }

    if ((data == nullptr && dataLen > 0) || position > entity_->capacity) {
        HILOGE("Invalid argument");
        return FsResult<size_t>::Error(EINVAL);
    }

    if (entity_->IsReadOnly()) {
        HILOGE("Read-only mmap buffer");
        return FsResult<size_t>::Error(FILEIO_SYS_CAP_TAG + E_MMAP_RO);
    }

    size_t remaining = (position < entity_->limit) ? (entity_->limit - position) : 0;
    size_t opLength = (length < remaining) ? length : remaining;
    if (opLength > dataLen) {
        opLength = dataLen;
    }

    if (opLength > 0) {
        int ret = memcpy_s(static_cast<char *>(entity_->mapAddr) + position,
            entity_->capacity - position, data, opLength);
        if (ret != 0) {
            HILOGE("Memcpy_s failed, error: %{public}d", ret);
            return FsResult<size_t>::Error(EIO);
        }
    }

    return FsResult<size_t>::Success(opLength);
}

FsResult<void> FsFileMapping::Msync(size_t offset, size_t length)
{
    FileFsTrace traceMsync("Msync");
    if (!CheckValid()) {
        return FsResult<void>::Error(EINVAL);
    }

    size_t rawOffset = entity_->adjustment + offset;
    if (rawOffset < entity_->adjustment) {
        HILOGE("Msync offset overflow");
        return FsResult<void>::Error(EINVAL);
    }
    if (rawOffset > entity_->rawCapacity) {
        HILOGE("Msync offset exceeds raw capacity");
        return FsResult<void>::Error(EINVAL);
    }
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        HILOGE("Failed to get system page size");
        return FsResult<void>::Error(EIO);
    }
    size_t alignedRawOffset = (rawOffset / static_cast<size_t>(pageSize)) * static_cast<size_t>(pageSize);
    size_t extra = rawOffset - alignedRawOffset;
    size_t rawLength = length + extra;
    if (rawLength < extra) {
        HILOGE("Msync length overflow");
        return FsResult<void>::Error(EINVAL);
    }
    if (alignedRawOffset + rawLength > entity_->rawCapacity) {
        HILOGE("Msync range exceeds raw capacity");
        return FsResult<void>::Error(EINVAL);
    }
    int ret = msync(static_cast<char *>(entity_->rawMapAddr) + alignedRawOffset, rawLength, MS_SYNC);
    if (ret < 0) {
        HILOGE("Failed to msync, error: %{public}d", errno);
        return FsResult<void>::Error(errno);
    }

    return FsResult<void>::Success();
}

FsResult<void> FsFileMapping::Unmap()
{
    FileFsTrace traceUnmap("Unmap");
    if (!entity_ || !entity_->isValid || entity_->rawMapAddr == nullptr) {
        return FsResult<void>::Success();
    }

    int ret = munmap(entity_->rawMapAddr, entity_->rawCapacity);
    if (ret < 0) {
        HILOGE("Failed to munmap, error: %{public}d", errno);
        return FsResult<void>::Error(errno);
    }

    entity_->mapAddr = nullptr;
    entity_->rawMapAddr = nullptr;
    entity_->isValid = false;
    entity_->capacity = 0;
    entity_->rawCapacity = 0;
    entity_->adjustment = 0;
    entity_->limit = 0;
    entity_->position = 0;

    return FsResult<void>::Success();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
