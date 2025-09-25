/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "uv_fs_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<UvFsMock> UvFsMock::uvfsMock = nullptr;
thread_local bool UvFsMock::mockable = false;

std::shared_ptr<UvFsMock> UvFsMock::GetMock()
{
    if (uvfsMock == nullptr) {
        uvfsMock = std::make_shared<UvFsMock>();
    }
    return uvfsMock;
}

void UvFsMock::EnableMock()
{
    mockable = true;
}

void UvFsMock::DisableMock()
{
    uvfsMock = nullptr;
    mockable = false;
}

bool UvFsMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int uv_fs_read(
    uv_loop_t *loop, uv_fs_t *req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t off, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_read(loop, req, file, bufs, nbufs, off, cb);
    }

    static int (*realUvfsRead)(
        uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb))dlsym(
            RTLD_NEXT, "uv_fs_read");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_read: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsRead) {
        return -1;
    }

    return realUvfsRead(loop, req, file, bufs, nbufs, off, cb);
}

int uv_fs_readlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_readlink(loop, req, path, cb);
    }

    static int (*realUvfsReadlink)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_readlink");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_readlink: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsReadlink) {
        return -1;
    }

    return realUvfsReadlink(loop, req, path, cb);
}

int uv_fs_stat(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_stat(loop, req, path, cb);
    }

    static int (*realUvfsStat)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_stat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_stat: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsStat) {
        return -1;
    }

    return realUvfsStat(loop, req, path, cb);
}

int uv_fs_utime(uv_loop_t *loop, uv_fs_t *req, const char *path, double atime, double mtime, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_utime(loop, req, path, atime, mtime, cb);
    }

    static int (*realUvfsUtime)(uv_loop_t *, uv_fs_t *, const char *, double, double, uv_fs_cb) = []() {
        auto func =
            (int (*)(uv_loop_t *, uv_fs_t *, const char *, double, double, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_utime");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_utime: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsUtime) {
        return -1;
    }

    return realUvfsUtime(loop, req, path, atime, mtime, cb);
}

int uv_fs_scandir(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_scandir(loop, req, path, flags, cb);
    }

    static int (*realUvfsScandir)(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_scandir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_scandir: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsScandir) {
        return -1;
    }

    return realUvfsScandir(loop, req, path, flags, cb);
}

int uv_fs_scandir_next(uv_fs_t *req, uv_dirent_t *ent)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_scandir_next(req, ent);
    }

    static int (*realUvfsScandirNext)(uv_fs_t *, uv_dirent_t *) = []() {
        auto func = (int (*)(uv_fs_t *, uv_dirent_t *))dlsym(RTLD_NEXT, "uv_fs_scandir_next");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_scandir_next: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsScandirNext) {
        return -1;
    }

    return realUvfsScandirNext(req, ent);
}

int uv_fs_rmdir(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_rmdir(loop, req, path, cb);
    }

    static int (*realUvfsRmdir)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_rmdir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_rmdir: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsRmdir) {
        return -1;
    }

    return realUvfsRmdir(loop, req, path, cb);
}

int uv_fs_symlink(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *new_path, int flags, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_symlink(loop, req, path, new_path, flags, cb);
    }

    static int (*realUvfsSymlink)(uv_loop_t *, uv_fs_t *, const char *, const char *, int flags, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, const char *, int flags, uv_fs_cb))dlsym(
            RTLD_NEXT, "uv_fs_symlink");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_symlink: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsSymlink) {
        return -1;
    }

    return realUvfsSymlink(loop, req, path, new_path, flags, cb);
}

int uv_fs_open(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, int mode, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_open(loop, req, path, flags, mode, cb);
    }

    static int (*realUvfsOpen)(uv_loop_t *, uv_fs_t *, const char *, int, int, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, int, int, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_open");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_open: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsOpen) {
        return -1;
    }

    return realUvfsOpen(loop, req, path, flags, mode, cb);
}

int uv_fs_ftruncate(uv_loop_t *loop, uv_fs_t *req, uv_file fd, int64_t offset, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_ftruncate(loop, req, fd, offset, cb);
    }

    static int (*realUvfsFtruncate)(uv_loop_t *, uv_fs_t *, uv_file, int64_t, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, int64_t, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_ftruncate");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_ftruncate: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsFtruncate) {
        return -1;
    }

    return realUvfsFtruncate(loop, req, fd, offset, cb);
}

int uv_fs_write(
    uv_loop_t *loop, uv_fs_t *req, uv_file fd, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_write(loop, req, fd, bufs, nbufs, offset, cb);
    }

    static int (*realUvfsWrite)(
        uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb))dlsym(
            RTLD_NEXT, "uv_fs_write");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_write: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsWrite) {
        return -1;
    }

    return realUvfsWrite(loop, req, fd, bufs, nbufs, offset, cb);
}

int uv_fs_realpath(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_realpath(loop, req, path, cb);
    }

    static int (*realUvfsRealpath)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_realpath");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_realpath: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsRealpath) {
        return -1;
    }

    return realUvfsRealpath(loop, req, path, cb);
}

int uv_fs_close(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_close(loop, req, file, cb);
    }

    static int (*realUvfsClose)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_close");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_close: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsClose) {
        return -1;
    }

    return realUvfsClose(loop, req, file, cb);
}

int uv_fs_fdatasync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_fdatasync(loop, req, file, cb);
    }

    static int (*realUvfsFdatasync)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_fdatasync");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_fdatasync: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsFdatasync) {
        return -1;
    }

    return realUvfsFdatasync(loop, req, file, cb);
}

int uv_fs_mkdir(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_mkdir(loop, req, path, mode, cb);
    }

    static int (*realUvfsMkdir)(uv_loop_t *, uv_fs_t *, const char *, int mode, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, int mode, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_mkdir");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_mkdir: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsMkdir) {
        return -1;
    }

    return realUvfsMkdir(loop, req, path, mode, cb);
}

int uv_fs_access(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_access(loop, req, path, flags, cb);
    }

    static int (*realUvfsAccess)(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_access");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_access: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsAccess) {
        return -1;
    }

    return realUvfsAccess(loop, req, path, flags, cb);
}

int uv_fs_mkdtemp(uv_loop_t *loop, uv_fs_t *req, const char *tpl, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_mkdtemp(loop, req, tpl, cb);
    }

    static int (*realUvfsMkdtemp)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_mkdtemp");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_mkdtemp: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsMkdtemp) {
        return -1;
    }

    return realUvfsMkdtemp(loop, req, tpl, cb);
}

int uv_fs_unlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_unlink(loop, req, path, cb);
    }

    static int (*realUvfsUnlink)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_unlink");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_unlink: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsUnlink) {
        return -1;
    }

    return realUvfsUnlink(loop, req, path, cb);
}

void uv_fs_req_cleanup(uv_fs_t *req)
{
    if (UvFsMock::IsMockable()) {
        UvFsMock::GetMock()->uv_fs_req_cleanup(req);
        return;
    }

    static void (*realUvfsCleanup)(uv_fs_t *) = []() {
        auto func = (void (*)(uv_fs_t *))dlsym(RTLD_NEXT, "uv_fs_req_cleanup");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_req_cleanup: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsCleanup) {
        return;
    }

    realUvfsCleanup(req);
}

int uv_fs_rename(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_rename(loop, req, path, newPath, cb);
    }

    static int (*realUvfsRename)(uv_loop_t *, uv_fs_t *, const char *, const char *, uv_fs_cb) = []() {
        auto func =
            (int (*)(uv_loop_t *, uv_fs_t *, const char *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_rename");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_rename: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsRename) {
        return -1;
    }

    return realUvfsRename(loop, req, path, newPath, cb);
}

int uv_fs_fsync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_fsync(loop, req, file, cb);
    }

    static int (*realUvfsFsync)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_fsync");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_fsync: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsFsync) {
        return -1;
    }

    return realUvfsFsync(loop, req, file, cb);
}

int uv_fs_sendfile(uv_loop_t *loop, uv_fs_t *req, uv_file outFd, uv_file inFd, int64_t off, size_t len, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_sendfile(loop, req, outFd, inFd, off, len, cb);
    }

    static int (*realUvfsSendfile)(uv_loop_t *, uv_fs_t *, uv_file, uv_file, int64_t, size_t, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_file, int64_t, size_t, uv_fs_cb))dlsym(
            RTLD_NEXT, "uv_fs_sendfile");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_sendfile: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsSendfile) {
        return -1;
    }

    return realUvfsSendfile(loop, req, outFd, inFd, off, len, cb);
}

int uv_fs_lstat(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_lstat(loop, req, path, cb);
    }

    static int (*realUvfsLstat)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_lstat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_lstat: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsLstat) {
        return -1;
    }

    return realUvfsLstat(loop, req, path, cb);
}

int uv_fs_fstat(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    if (UvFsMock::IsMockable()) {
        return UvFsMock::GetMock()->uv_fs_fstat(loop, req, file, cb);
    }

    static int (*realUvfsFstat)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_fstat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_fstat: " << dlerror();
        }
        return func;
    }();

    if (!realUvfsFstat) {
        return -1;
    }

    return realUvfsFstat(loop, req, file, cb);
}
} // extern "C"
#endif