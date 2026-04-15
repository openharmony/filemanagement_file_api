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
 
#include "fdtag_func.h"
#include "filemgmt_libhilog.h"
 
#include <stdatomic.h>
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
#include <sys/mman.h>
#endif
 
namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
#define ALIGN_SIZE 4096
inline size_t FS_ALIGN(size_t x, size_t y)
{
    if (y == 0) {
        return 0;
    }
    return ((x + y - 1) / y) * y;
}

static struct FdSanTable g_fdTable = {
    .overflow = nullptr,
};
 
static struct FdSanEntry* GetFsFdEntry(size_t idx)
{
    if (idx >= FD_SAN_OVERFLOW_END) {
        return nullptr;
    }
    struct FdSanEntry *entries = g_fdTable.entries;
    if (idx < FD_SAN_TABLE_SIZE) {
        return &entries[idx];
    }
	// Try to create the overflow table ourselves.
    struct FdSanTableOverflow* localOverflow = atomic_load(&g_fdTable.overflow);
    if (!localOverflow) {
        size_t overflowCount = FD_SAN_OVERFLOW_END - FD_SAN_TABLE_SIZE;
        size_t requiredSize = sizeof(struct FdSanTableOverflow) + overflowCount * sizeof(struct FdSanEntry);
        size_t alignedSize = FS_ALIGN(requiredSize, ALIGN_SIZE);
 
        size_t alignedCount = (alignedSize - sizeof(struct FdSanTableOverflow)) / sizeof(struct FdSanEntry);
        void* allocation =
                mmap(nullptr, alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (allocation == MAP_FAILED) {
            HILOGE("fdsan: mmap failed idx=%{public}zu", idx);
            localOverflow = atomic_load(&g_fdTable.overflow);
            if (!localOverflow) {
                return nullptr;
            }
        }
        struct FdSanTableOverflow* newOverflow = (struct FdSanTableOverflow*)(allocation);
        newOverflow->len = alignedCount;
        if (atomic_compare_exchange_strong(&g_fdTable.overflow, &localOverflow, newOverflow)) {
            localOverflow = newOverflow;
        } else {
            // Another thread had mmaped.
            munmap(allocation, alignedSize);
        }
    }
 
    size_t offset = idx - FD_SAN_TABLE_SIZE;
    if (localOverflow->len <= offset) {
        return nullptr;
    }
    return &localOverflow->entries[offset];
}
 
static struct FdSanEntry* GetFdSanEntry(int fd)
{
    if (fd < 0) {
        return nullptr;
    }
    return GetFsFdEntry(fd);
}
 
uint64_t FdTagFunc::GetFdTag(int fd)
{
    struct FdSanEntry* fde = GetFdSanEntry(fd);
    if (!fde) {
        return 0;
    }
    return atomic_load(&fde->closeTag);
}
 
void FdTagFunc::SetFdTag(int fd, uint64_t tag)
{
    struct FdSanEntry* fde = GetFdSanEntry(fd);
    if (!fde) {
        return;
    }
    atomic_store(&fde->closeTag, tag);
}
#endif
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS