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

#include "swapfs_err_mapper.h"
#include "swapfs_io_engine.h"

#include "filemgmt_libhilog.h"

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "swapfs_manager.h"

#ifndef O_DIRECT
#define O_DIRECT 0
#endif

namespace OHOS::FileManagement::Swapfs {
namespace {
constexpr mode_t SWAP_FILE_MODE = S_IRUSR | S_IWUSR;

int WriteFull(int fd, const void *buffer, size_t size)
{
    const char *cursor = static_cast<const char *>(buffer);
    size_t remaining = size;
    while (remaining > 0) {
        ssize_t ret = write(fd, cursor, remaining);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            HILOGE("[Swapfs] write failed, errno: %{public}d", errno);
            return MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
        }
        if (ret == 0) {
            return SWAPFS_E_IO_ERROR;
        }
        cursor += ret;
        remaining -= static_cast<size_t>(ret);
    }
    return SWAPFS_E_OK;
}

int ReadFull(int fd, void *buffer, size_t size, size_t offset)
{
    char *cursor = static_cast<char *>(buffer);
    size_t remaining = size;
    off_t fileOffset = static_cast<off_t>(offset);
    while (remaining > 0) {
        ssize_t ret = pread(fd, cursor, remaining, fileOffset);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            HILOGE("[Swapfs] pread failed, errno: %{public}d", errno);
            return MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
        }
        if (ret == 0) {
            return SWAPFS_E_IO_ERROR;
        }
        cursor += ret;
        remaining -= static_cast<size_t>(ret);
        fileOffset += ret;
    }
    return SWAPFS_E_OK;
}
} // namespace

bool IsDioAligned(const void *buffer, size_t size)
{
    return buffer != nullptr && (reinterpret_cast<uintptr_t>(buffer) % DIO_ALIGNMENT) == 0 &&
        (size % DIO_ALIGNMENT) == 0;
}

int SyncReadEngine::Read(
    const std::string &path, void *buffer, size_t size, size_t offset, bool useDirectIo)
{
    if (buffer == nullptr || size == 0) {
        HILOGW("[Swapfs] Read invalid params");
        return SWAPFS_E_INVAL;
    }
    if (useDirectIo && !IsDioAligned(buffer, size)) {
        HILOGW("[Swapfs] Read DIO alignment check failed");
        return SWAPFS_E_DIO_ALIGN;
    }
    int flags = O_RDONLY | O_CLOEXEC;
    if (useDirectIo) {
        flags |= O_DIRECT;
    }
    int fd = open(path.c_str(), flags);
    if (fd < 0) {
        HILOGE("[Swapfs] Read open failed, errno: %{public}d", errno);
        return MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
    }
    int ret = ReadFull(fd, buffer, size, offset);
    (void)close(fd);
    return ret;
}

int SyncWriteEngine::Write(
    const std::string &path, const void *buffer, size_t size, bool useDirectIo)
{
    if (buffer == nullptr || size == 0) {
        HILOGW("[Swapfs] Write invalid params");
        return SWAPFS_E_INVAL;
    }
    int flags = O_CREAT | O_CLOEXEC | O_TRUNC | O_WRONLY;
    if (useDirectIo) {
        flags |= O_DIRECT;
    }
    int fd = open(path.c_str(), flags, SWAP_FILE_MODE);
    if (fd < 0) {
        HILOGE("[Swapfs] Write open failed, errno: %{public}d", errno);
        return MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
    }

    int ret = WriteFull(fd, buffer, size);
    if (ret == SWAPFS_E_OK && fsync(fd) != 0) {
        HILOGE("[Swapfs] Write fsync failed, errno: %{public}d", errno);
        ret = MapErrno(errno, SwapfsErrContext::KEY_OPERATION);
    }
    (void)close(fd);
    return ret;
}
} // namespace OHOS::FileManagement::Swapfs
