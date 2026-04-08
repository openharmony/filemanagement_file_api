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

#include "mmap_core.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <unistd.h>

#include <optional>

#include "file_fs_trace.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

std::tuple<int, int> MmapCore::GetMmapProtFlags(int mode)
{
    int prot = PROT_READ;
    int flags = MAP_SHARED;
    switch (mode) {
        case MappingMode::READ_ONLY:
            prot = PROT_READ;
            flags = MAP_SHARED;
            break;
        case MappingMode::READ_WRITE:
            prot = PROT_READ | PROT_WRITE;
            flags = MAP_SHARED;
            break;
        case MappingMode::PRIVATE:
            prot = PROT_READ | PROT_WRITE;
            flags = MAP_PRIVATE;
            break;
        default:
            break;
    }
    return { prot, flags };
}

int MmapCore::ValidateFile(int fd)
{
    struct stat st;
    if (fstat(fd, &st) < 0) {
        HILOGE("Failed to fstat, error: %{public}d", errno);
        return errno;
    }

    if (!S_ISREG(st.st_mode) && !S_ISBLK(st.st_mode)) {
        HILOGE("File type does not support mmap, mode: %{public}u", st.st_mode);
        return FILEIO_SYS_CAP_TAG + E_MMAP_FILE;
    }

    return 0;
}

static constexpr long FS_TYPE_EXT4 = 0xEF53;
static constexpr long FS_TYPE_HMDFS_LOCAL = 0x20200302;
static constexpr long FS_TYPE_F2FS = 0xF2F52010;

int MmapCore::ValidateFilesystem(int fd)
{
    struct statfs fsInfo;
    if (fstatfs(fd, &fsInfo) < 0) {
        HILOGE("Failed to fstatfs, error: %{public}d", errno);
        return FILEIO_SYS_CAP_TAG + E_MMAP_UNSUP;
    }
    long fsType = static_cast<long>(fsInfo.f_type);
    if (fsType != FS_TYPE_EXT4 && fsType != FS_TYPE_HMDFS_LOCAL && fsType != FS_TYPE_F2FS) {
        HILOGE("Unsupported filesystem type: %{public}lx", static_cast<unsigned long>(fsType));
        return FILEIO_SYS_CAP_TAG + E_MMAP_UNSUP;
    }
    return 0;
}

struct PageAlignResult {
    off_t alignedOffset;
    size_t adjustment;
    size_t adjustedSize;
};

static std::optional<PageAlignResult> AlignToPage(off_t offset, size_t size)
{
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return std::nullopt;
    }
    off_t pageSz = static_cast<off_t>(pageSize);
    off_t alignedOffset = (offset / pageSz) * pageSz;
    size_t adjustment = static_cast<size_t>(offset - alignedOffset);
    size_t adjustedSize = size + adjustment;
    return PageAlignResult{ alignedOffset, adjustment, adjustedSize };
}

int MmapCore::ExpandFileIfNeeded(int fd, int mode, off_t offset, size_t size)
{
    struct stat st;
    if (fstat(fd, &st) < 0) {
        HILOGE("Failed to fstat, error: %{public}d", errno);
        return errno;
    }

    off_t requiredSize = offset + static_cast<off_t>(size);
    if (requiredSize <= st.st_size) {
        return 0;
    }

    if (ftruncate(fd, requiredSize) < 0) {
        int truncateErr = errno;
        HILOGE("Failed to expand file size, error: %{public}d", truncateErr);
        if (S_ISREG(st.st_mode) || truncateErr != EINVAL) {
            return truncateErr;
        }
    }

    return 0;
}

static int ValidateMmapParams(int fd, int mode, off_t offset, size_t size)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return EBADF;
    }
    if (mode < 0 || mode > MappingMode::PRIVATE) {
        HILOGE("Invalid mode value");
        return EINVAL;
    }
    if (offset < 0) {
        HILOGE("Invalid offset value");
        return EINVAL;
    }
    if (size <= 0) {
        HILOGE("Invalid size value");
        return EINVAL;
    }
    return 0;
}

FsResult<FsFileMapping *> MmapCore::DoMmap(int fd, int mode, off_t offset, size_t size)
{
    FileFsTrace traceMmap("DoMmap");

    int err = ValidateMmapParams(fd, mode, offset, size);
    if (err != 0) {
        return FsResult<FsFileMapping *>::Error(err);
    }

    int validateResult = ValidateFile(fd);
    if (validateResult != 0) {
        return FsResult<FsFileMapping *>::Error(validateResult);
    }

    int fsResult = ValidateFilesystem(fd);
    if (fsResult != 0) {
        return FsResult<FsFileMapping *>::Error(fsResult);
    }

    auto alignResult = AlignToPage(offset, size);
    if (!alignResult.has_value()) {
        HILOGE("Failed to get system page size");
        return FsResult<FsFileMapping *>::Error(EIO);
    }
    off_t alignedOffset = alignResult->alignedOffset;
    size_t adjustment = alignResult->adjustment;
    size_t adjustedSize = alignResult->adjustedSize;

    int expandResult = ExpandFileIfNeeded(fd, mode, alignedOffset, adjustedSize);
    if (expandResult != 0) {
        return FsResult<FsFileMapping *>::Error(expandResult);
    }

    auto [prot, flags] = GetMmapProtFlags(mode);
    void *rawMapAddr = mmap(nullptr, adjustedSize, prot, flags, fd, alignedOffset);
    if (rawMapAddr == MAP_FAILED) {
        HILOGE("Failed to mmap, error: %{public}d", errno);
        return FsResult<FsFileMapping *>::Error(errno);
    }

    void *mapAddr = static_cast<char *>(rawMapAddr) + adjustment;

    FileMappingParams params = { mapAddr, rawMapAddr, size, adjustedSize, adjustment, mode, fd, offset };
    auto result = FsFileMapping::Constructor(params);
    if (!result.IsSuccess()) {
        munmap(rawMapAddr, adjustedSize);
        return result;
    }

    return result;
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
