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

#include "swapfs_syscall_mock.h"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <utility>

namespace {
bool OpenNeedsMode(int flags)
{
    return (flags & O_CREAT) != 0;
}
} // namespace

namespace OHOS::FileManagement::Swapfs::Test {

thread_local std::shared_ptr<SwapfsSyscallMock> SwapfsSyscallMock::mock_ = nullptr;
thread_local bool SwapfsSyscallMock::mockable_ = false;

namespace {
void ConfigureRenameMocks(const std::shared_ptr<SwapfsSyscallMock> &mock)
{
    ON_CALL(*mock, Rename(testing::_, testing::_))
        .WillByDefault(testing::Invoke([](const char *oldPath, const char *newPath) {
            using Func = int (*)(const char *, const char *);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "rename"));
            return real ? real(oldPath, newPath) : -1;
        }));
    ON_CALL(*mock, RenameAt(testing::_, testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke(
            [](int oldDirFd, const char *oldPath, int newDirFd, const char *newPath) {
                using Func = int (*)(int, const char *, int, const char *);
                static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "renameat"));
                return real ? real(oldDirFd, oldPath, newDirFd, newPath) : -1;
            }));
}

void ConfigureAtMocks(const std::shared_ptr<SwapfsSyscallMock> &mock)
{
    ON_CALL(*mock, OpenAt(testing::_, testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke([](int dirFd, const char *path, int flags, mode_t mode) {
            return static_cast<int>(syscall(SYS_openat, dirFd, path, flags, mode));
        }));
    ON_CALL(*mock, MkdirAt(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke([](int dirFd, const char *path, mode_t mode) {
            return static_cast<int>(syscall(SYS_mkdirat, dirFd, path, mode));
        }));
}
} // namespace

std::shared_ptr<SwapfsSyscallMock> SwapfsSyscallMock::GetMock()
{
    if (mock_ == nullptr) {
        mock_ = std::make_shared<SwapfsSyscallMock>();
    }
    return mock_;
}

void SwapfsSyscallMock::EnableMock()
{
    mockable_ = true;
    auto mock = GetMock();
    ON_CALL(*mock, Write(testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke([](int fd, const void *buf, size_t size) {
            using Func = ssize_t (*)(int, const void *, size_t);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "write"));
            return real ? real(fd, buf, size) : -1;
        }));
    ON_CALL(*mock, Pread(testing::_, testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke([](int fd, void *buf, size_t size, off_t offset) {
            using Func = ssize_t (*)(int, void *, size_t, off_t);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "pread"));
            return real ? real(fd, buf, size, offset) : -1;
        }));
    ON_CALL(*mock, Fsync(testing::_))
        .WillByDefault(testing::Invoke([](int fd) {
            using Func = int (*)(int);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "fsync"));
            return real ? real(fd) : -1;
        }));
    ON_CALL(*mock, Unlink(testing::_))
        .WillByDefault(testing::Invoke([](const char *path) {
            using Func = int (*)(const char *);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "unlink"));
            return real ? real(path) : -1;
        }));
    ConfigureRenameMocks(mock);
    ConfigureAtMocks(mock);
    ON_CALL(*mock, Flock(testing::_, testing::_))
        .WillByDefault(testing::Invoke([](int fd, int operation) {
            using Func = int (*)(int, int);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "flock"));
            return real ? real(fd, operation) : -1;
        }));
    ON_CALL(*mock, Mkdir(testing::_, testing::_))
        .WillByDefault(testing::Invoke([](const char *path, mode_t mode) {
            using Func = int (*)(const char *, mode_t);
            static Func real = reinterpret_cast<Func>(dlsym(RTLD_NEXT, "mkdir"));
            return real ? real(path, mode) : -1;
        }));
    ON_CALL(*mock, Dup(testing::_))
        .WillByDefault(testing::Invoke([](int oldFd) {
            return static_cast<int>(syscall(SYS_dup, oldFd));
        }));
}

void SwapfsSyscallMock::DisableMock()
{
    mockable_ = false;
    mock_ = nullptr;
}

bool SwapfsSyscallMock::IsMockable()
{
    return mockable_;
}

} // namespace OHOS::FileManagement::Swapfs::Test

namespace {
using SwapfsSyscallMock = OHOS::FileManagement::Swapfs::Test::SwapfsSyscallMock;

template<typename Function>
Function ResolveFunction(const char *name)
{
    Function function = reinterpret_cast<Function>(dlsym(RTLD_NEXT, name));
    if (function == nullptr) {
        errno = ENOSYS;
    }
    return function;
}
} // namespace

extern "C" {
ssize_t write(int fd, const void *buffer, size_t size)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Write(fd, buffer, size);
    }
    using WriteFunction = ssize_t (*)(int, const void *, size_t);
    static WriteFunction realWrite = ResolveFunction<WriteFunction>("write");
    return realWrite == nullptr ? -1 : realWrite(fd, buffer, size);
}

ssize_t pread(int fd, void *buffer, size_t size, off_t offset)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Pread(fd, buffer, size, offset);
    }
    using PreadFunction = ssize_t (*)(int, void *, size_t, off_t);
    static PreadFunction realPread = ResolveFunction<PreadFunction>("pread");
    return realPread == nullptr ? -1 : realPread(fd, buffer, size, offset);
}

int fsync(int fd)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Fsync(fd);
    }
    using FsyncFunction = int (*)(int);
    static FsyncFunction realFsync = ResolveFunction<FsyncFunction>("fsync");
    return realFsync == nullptr ? -1 : realFsync(fd);
}

int unlink(const char *path)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Unlink(path);
    }
    using UnlinkFunction = int (*)(const char *);
    static UnlinkFunction realUnlink = ResolveFunction<UnlinkFunction>("unlink");
    return realUnlink == nullptr ? -1 : realUnlink(path);
}

int rename(const char *oldpath, const char *newpath)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Rename(oldpath, newpath);
    }
    using RenameFunction = int (*)(const char *, const char *);
    static RenameFunction realRename = ResolveFunction<RenameFunction>("rename");
    return realRename == nullptr ? -1 : realRename(oldpath, newpath);
}

int renameat(int oldDirFd, const char *oldPath, int newDirFd, const char *newPath)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->RenameAt(
            oldDirFd, oldPath, newDirFd, newPath);
    }
    using RenameAtFunction = int (*)(int, const char *, int, const char *);
    static RenameAtFunction realRenameAt = ResolveFunction<RenameAtFunction>("renameat");
    return realRenameAt == nullptr ?
        -1 : realRenameAt(oldDirFd, oldPath, newDirFd, newPath);
}

int openat(int dirFd, const char *path, int flags, ...)
{
    mode_t mode = 0;
    if (OpenNeedsMode(flags)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->OpenAt(dirFd, path, flags, mode);
    }
    return static_cast<int>(syscall(SYS_openat, dirFd, path, flags, mode));
}

int mkdirat(int dirFd, const char *path, mode_t mode)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->MkdirAt(dirFd, path, mode);
    }
    return static_cast<int>(syscall(SYS_mkdirat, dirFd, path, mode));
}

int flock(int fd, int operation)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Flock(fd, operation);
    }
    using FlockFunction = int (*)(int, int);
    static FlockFunction realFlock = ResolveFunction<FlockFunction>("flock");
    return realFlock == nullptr ? -1 : realFlock(fd, operation);
}

int mkdir(const char *path, mode_t mode)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Mkdir(path, mode);
    }
    using MkdirFunction = int (*)(const char *, mode_t);
    static MkdirFunction realMkdir = ResolveFunction<MkdirFunction>("mkdir");
    return realMkdir == nullptr ? -1 : realMkdir(path, mode);
}

int dup(int oldFd)
{
    if (SwapfsSyscallMock::IsMockable()) {
        return SwapfsSyscallMock::GetMock()->Dup(oldFd);
    }
    return static_cast<int>(syscall(SYS_dup, oldFd));
}

} // extern "C"
