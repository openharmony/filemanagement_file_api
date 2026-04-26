/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "mmap_mock.h"

#include <dlfcn.h>
#include <securec.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<MmapMock> MmapMock::mmapMock_ = nullptr;
thread_local bool MmapMock::mockable_ = false;

std::shared_ptr<MmapMock> MmapMock::GetMock()
{
    if (mmapMock_ == nullptr) {
        mmapMock_ = std::make_shared<MmapMock>();
    }
    return mmapMock_;
}

void MmapMock::EnableMock()
{
    mockable_ = true;
}

void MmapMock::DisableMock()
{
    mmapMock_ = nullptr;
    mockable_ = false;
}

bool MmapMock::IsMockable()
{
    return mockable_;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using MmapMock = OHOS::FileManagement::ModuleFileIO::Test::MmapMock;

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->mmap(addr, length, prot, flags, fd, offset);
    }

    static void *(*realMmap)(void *, size_t, int, int, int, off_t) = nullptr;
    if (!realMmap) {
        realMmap = (void *(*)(void *, size_t, int, int, int, off_t))dlsym(RTLD_NEXT, "mmap");
        if (!realMmap) {
            GTEST_LOG_(ERROR) << "Failed to resolve real mmap: " << dlerror();
            return MAP_FAILED;
        }
    }
    return realMmap(addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->munmap(addr, length);
    }

    static int (*realMunmap)(void *, size_t) = nullptr;
    if (!realMunmap) {
        realMunmap = (int (*)(void *, size_t))dlsym(RTLD_NEXT, "munmap");
        if (!realMunmap) {
            GTEST_LOG_(ERROR) << "Failed to resolve real munmap: " << dlerror();
            return -1;
        }
    }
    return realMunmap(addr, length);
}

int msync(void *addr, size_t length, int flags)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->msync(addr, length, flags);
    }

    static int (*realMsync)(void *, size_t, int) = nullptr;
    if (!realMsync) {
        realMsync = (int (*)(void *, size_t, int))dlsym(RTLD_NEXT, "msync");
        if (!realMsync) {
            GTEST_LOG_(ERROR) << "Failed to resolve real msync: " << dlerror();
            return -1;
        }
    }
    return realMsync(addr, length, flags);
}

int fstat(int fd, struct stat *statbuf)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->fstat(fd, statbuf);
    }

    static int (*realFstat)(int, struct stat *) = nullptr;
    if (!realFstat) {
        realFstat = (int (*)(int, struct stat *))dlsym(RTLD_NEXT, "fstat");
        if (!realFstat) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fstat: " << dlerror();
            return -1;
        }
    }
    return realFstat(fd, statbuf);
}

int ftruncate(int fd, off_t length)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->ftruncate(fd, length);
    }

    static int (*realFtruncate)(int, off_t) = nullptr;
    if (!realFtruncate) {
        realFtruncate = (int (*)(int, off_t))dlsym(RTLD_NEXT, "ftruncate");
        if (!realFtruncate) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ftruncate: " << dlerror();
            return -1;
        }
    }
    return realFtruncate(fd, length);
}

long sysconf(int name)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->sysconf(name);
    }

    static long (*realSysconf)(int) = nullptr;
    if (!realSysconf) {
        realSysconf = (long (*)(int))dlsym(RTLD_NEXT, "sysconf");
        if (!realSysconf) {
            GTEST_LOG_(ERROR) << "Failed to resolve real sysconf: " << dlerror();
            return -1;
        }
    }
    return realSysconf(name);
}

errno_t memcpy_s(void *dest, size_t destMax, const void *src, size_t count)
{
    if (MmapMock::IsMockable()) {
        return MmapMock::GetMock()->memcpy_s(dest, destMax, src, count);
    }

    static errno_t (*realMemcpyS)(void *, size_t, const void *, size_t) = nullptr;
    if (!realMemcpyS) {
        realMemcpyS = (errno_t(*)(void *, size_t, const void *, size_t))dlsym(RTLD_NEXT, "memcpy_s");
        if (!realMemcpyS) {
            GTEST_LOG_(ERROR) << "Failed to resolve real memcpy_s: " << dlerror();
            return ERANGE;
        }
    }
    return realMemcpyS(dest, destMax, src, count);
}

#ifdef __cplusplus
}
#endif
