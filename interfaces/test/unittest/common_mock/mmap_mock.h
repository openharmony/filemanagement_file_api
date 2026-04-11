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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_MMAP_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_MMAP_MOCK_H

#include <sys/mman.h>
#include <sys/stat.h>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IMmapMock {
public:
    virtual ~IMmapMock() = default;
    virtual void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) = 0;
    virtual int munmap(void *addr, size_t length) = 0;
    virtual int msync(void *addr, size_t length, int flags) = 0;
    virtual int fstat(int fd, struct stat *statbuf) = 0;
    virtual int ftruncate(int fd, off_t length) = 0;
    virtual long sysconf(int name) = 0;
};

class MmapMock : public IMmapMock {
public:
    MOCK_METHOD(void*, mmap, (void *addr, size_t length, int prot, int flags, int fd, off_t offset), (override));
    MOCK_METHOD(int, munmap, (void *addr, size_t length), (override));
    MOCK_METHOD(int, msync, (void *addr, size_t length, int flags), (override));
    MOCK_METHOD(int, fstat, (int fd, struct stat *statbuf), (override));
    MOCK_METHOD(int, ftruncate, (int fd, off_t length), (override));
    MOCK_METHOD(long, sysconf, (int name), (override));

public:
    static std::shared_ptr<MmapMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<MmapMock> mmapMock_;
    static thread_local bool mockable_;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_MMAP_MOCK_H
