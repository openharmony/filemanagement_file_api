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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_SWAPFS_SYSCALL_MOCK_H
#define OHOS_FILEMANAGEMENT_FILE_API_SWAPFS_SYSCALL_MOCK_H

#include <cstddef>
#include <memory>
#include <sys/types.h>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::Swapfs::Test {

class SwapfsSyscallMock {
public:
    MOCK_METHOD(ssize_t, Write, (int fd, const void *buffer, size_t size));
    MOCK_METHOD(ssize_t, Pread, (int fd, void *buffer, size_t size, off_t offset));
    MOCK_METHOD(int, Fsync, (int fd));
    MOCK_METHOD(int, Unlink, (const char *path));
    MOCK_METHOD(int, Rename, (const char *oldpath, const char *newpath));
    MOCK_METHOD(int, RenameAt,
        (int oldDirFd, const char *oldPath, int newDirFd, const char *newPath));
    MOCK_METHOD(int, OpenAt, (int dirFd, const char *path, int flags, mode_t mode));
    MOCK_METHOD(int, MkdirAt, (int dirFd, const char *path, mode_t mode));
    MOCK_METHOD(int, Flock, (int fd, int operation));
    MOCK_METHOD(int, Mkdir, (const char *path, mode_t mode));
    MOCK_METHOD(int, Dup, (int oldFd));

    static std::shared_ptr<SwapfsSyscallMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<SwapfsSyscallMock> mock_;
    static thread_local bool mockable_;
};

} // namespace OHOS::FileManagement::Swapfs::Test

#endif // OHOS_FILEMANAGEMENT_FILE_API_SWAPFS_SYSCALL_MOCK_H
