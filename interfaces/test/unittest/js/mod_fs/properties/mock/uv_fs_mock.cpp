/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "uv_fs_mock.h"

using namespace OHOS::FileManagement::ModuleFileIO;

int uv_fs_read(uv_loop_t *loop, uv_fs_t *req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t off,
    uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_read(loop, req, file, bufs, nbufs, off, cb);
}

int uv_fs_readlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_readlink(loop, req, path, cb);
}

int uv_fs_stat(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_stat(loop, req, path, cb);
}

int uv_fs_utime(uv_loop_t *loop, uv_fs_t *req, const char *path, double atime, double mtime, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_utime(loop, req, path, atime, mtime, cb);
}

int uv_fs_scandir(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_scandir(loop, req, path, flags, cb);
}

int uv_fs_scandir_next(uv_fs_t *req, uv_dirent_t *ent)
{
    return Uvfs::ins->uv_fs_scandir_next(req, ent);
}

int uv_fs_rmdir(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_rmdir(loop, req, path, cb);
}

int uv_fs_symlink(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, int flags, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_symlink(loop, req, path, newPath, flags, cb);
}

int uv_fs_open(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, int mode, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_open(loop, req, path, flags, mode, cb);
}

int uv_fs_ftruncate(uv_loop_t *loop, uv_fs_t *req, uv_file fd, int64_t offset, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_ftruncate(loop, req, fd, offset, cb);
}

int uv_fs_write(uv_loop_t *loop, uv_fs_t *req, uv_file fd, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset,
    uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_write(loop, req, fd, bufs, nbufs, offset, cb);
}

int uv_fs_realpath(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_realpath(loop, req, path, cb);
}

int uv_fs_close(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_close(loop, req, file, cb);
}

int uv_fs_fdatasync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_fdatasync(loop, req, file, cb);
}

int uv_fs_mkdir(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_mkdir(loop, req, path, mode, cb);
}

int uv_fs_access(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_access(loop, req, path, flags, cb);
}

int uv_fs_mkdtemp(uv_loop_t *loop, uv_fs_t *req, const char *tpl, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_mkdtemp(loop, req, tpl, cb);
}

int uv_fs_unlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_unlink(loop, req, path, cb);
}

void uv_fs_req_cleanup(uv_fs_t *req)
{
    return;
}

int uv_fs_rename(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_rename(loop, req, path, newPath, cb);
}

int uv_fs_fsync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_fsync(loop, req, file, cb);
}

int uv_fs_sendfile(uv_loop_t *loop, uv_fs_t *req, uv_file outFd, uv_file inFd, int64_t off, size_t len, uv_fs_cb cb)
{
    return Uvfs::ins->uv_fs_sendfile(loop, req, outFd, inFd, off, len, cb);
}
