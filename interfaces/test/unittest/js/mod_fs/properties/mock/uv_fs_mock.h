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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H

#include "uv.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO {

class Uvfs {
public:
    static inline std::shared_ptr<Uvfs> ins = nullptr;

public:
    virtual ~Uvfs() = default;
    virtual int uv_fs_read(uv_loop_t *loop, uv_fs_t *req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs,
        int64_t off, uv_fs_cb cb) = 0;
    virtual int uv_fs_readlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_stat(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_utime(uv_loop_t *loop, uv_fs_t *req, const char *path, double atime,
        double mtime, uv_fs_cb cb) = 0;
    virtual int uv_fs_scandir(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb) = 0;
    virtual int uv_fs_scandir_next(uv_fs_t *req, uv_dirent_t *ent) = 0;
    virtual int uv_fs_rmdir(uv_loop_t *loop, uv_fs_t *req, const char* path, uv_fs_cb cb) = 0;
    virtual int uv_fs_symlink(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, int flags,
        uv_fs_cb cb) = 0;
    virtual int uv_fs_open(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, int mode, uv_fs_cb cb) = 0;
    virtual int uv_fs_ftruncate(uv_loop_t *loop, uv_fs_t *req, uv_file fd, int64_t offset, uv_fs_cb cb) = 0;
    virtual int uv_fs_write(uv_loop_t *loop, uv_fs_t *req, uv_file fd, const uv_buf_t bufs[], unsigned int nbufs,
        int64_t offset, uv_fs_cb cb) = 0;
    virtual int uv_fs_realpath(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_close(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb) = 0;
    virtual int uv_fs_fdatasync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb) = 0;
    virtual int uv_fs_mkdir(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb) = 0;
    virtual int uv_fs_access(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb) = 0;
    virtual int uv_fs_mkdtemp(uv_loop_t *loop, uv_fs_t *req, const char *tpl, uv_fs_cb cb) = 0;
    virtual int uv_fs_unlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_rename(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, uv_fs_cb cb) = 0;
    virtual void uv_fs_req_cleanup(uv_fs_t *req) = 0;
    virtual int uv_fs_fsync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb) = 0;
    virtual int uv_fs_sendfile(uv_loop_t *loop, uv_fs_t *req, uv_file outFd, uv_file inFd, int64_t off, size_t len,
        uv_fs_cb cb) = 0;
};

class UvfsMock : public Uvfs {
public:
    MOCK_METHOD7(uv_fs_read, int(uv_loop_t *loop, uv_fs_t *req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs,
        int64_t off, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_readlink, int(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_stat, int(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb));
    MOCK_METHOD6(uv_fs_utime, int(uv_loop_t *loop, uv_fs_t *req, const char *path, double atime, double mtime,
        uv_fs_cb cb));
    MOCK_METHOD5(uv_fs_scandir, int(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb));
    MOCK_METHOD2(uv_fs_scandir_next, int(uv_fs_t *req, uv_dirent_t *ent));
    MOCK_METHOD4(uv_fs_rmdir, int(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb));
    MOCK_METHOD6(uv_fs_symlink, int(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, int flags,
        uv_fs_cb cb));
    MOCK_METHOD6(uv_fs_open, int(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, int mode, uv_fs_cb cb));
    MOCK_METHOD5(uv_fs_ftruncate, int(uv_loop_t *loop, uv_fs_t *req, uv_file fd, int64_t offset, uv_fs_cb cb));
    MOCK_METHOD7(uv_fs_write, int(uv_loop_t *loop, uv_fs_t *req, uv_file fd, const uv_buf_t bufs[], unsigned int nbufs,
        int64_t offset, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_realpath, int(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_close, int(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_fdatasync, int(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb));
    MOCK_METHOD5(uv_fs_mkdir, int(uv_loop_t *loop, uv_fs_t *req, const char *path, int mode, uv_fs_cb cb));
    MOCK_METHOD5(uv_fs_access, int(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_mkdtemp, int(uv_loop_t *loop, uv_fs_t *req, const char *tpl, uv_fs_cb cb));
    MOCK_METHOD4(uv_fs_unlink, int(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb));
    MOCK_METHOD5(uv_fs_rename, int(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, uv_fs_cb cb));
    MOCK_METHOD1(uv_fs_req_cleanup, void(uv_fs_t *req));
    MOCK_METHOD4(uv_fs_fsync, int(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb));
    MOCK_METHOD7(uv_fs_sendfile, int(uv_loop_t *loop, uv_fs_t *req, uv_file outFd, uv_file inFd, int64_t off,
        size_t len, uv_fs_cb cb));
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H