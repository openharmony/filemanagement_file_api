/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this  except in compliance with the License.
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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_FS_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_FS_MOCK_H

#include "uv.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IUvFs {
public:
    virtual ~IUvFs() = default;
    virtual int uv_fs_read(uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb) = 0;
    virtual int uv_fs_readlink(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_stat(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_utime(uv_loop_t *, uv_fs_t *, const char *, double, double, uv_fs_cb) = 0;
    virtual int uv_fs_scandir(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb) = 0;
    virtual int uv_fs_scandir_next(uv_fs_t *, uv_dirent_t *) = 0;
    virtual int uv_fs_rmdir(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_symlink(uv_loop_t *, uv_fs_t *, const char *, const char *, int, uv_fs_cb) = 0;
    virtual int uv_fs_open(uv_loop_t *, uv_fs_t *, const char *, int, int, uv_fs_cb) = 0;
    virtual int uv_fs_ftruncate(uv_loop_t *, uv_fs_t *, uv_file, int64_t, uv_fs_cb) = 0;
    virtual int uv_fs_write(uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb) = 0;
    virtual int uv_fs_realpath(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_close(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = 0;
    virtual int uv_fs_fdatasync(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = 0;
    virtual int uv_fs_mkdir(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb) = 0;
    virtual int uv_fs_access(uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb) = 0;
    virtual int uv_fs_mkdtemp(uv_loop_t *, uv_fs_t *, const char *tpl, uv_fs_cb) = 0;
    virtual int uv_fs_unlink(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_rename(uv_loop_t *, uv_fs_t *, const char *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_fsync(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = 0;
    virtual int uv_fs_sendfile(uv_loop_t *, uv_fs_t *, uv_file, uv_file, int64_t, size_t, uv_fs_cb) = 0;
    virtual int uv_fs_lstat(uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb) = 0;
    virtual int uv_fs_fstat(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = 0;
    virtual void uv_fs_req_cleanup(uv_fs_t *) = 0;
};

class UvFsMock : public IUvFs {
public:
    MOCK_METHOD(int, uv_fs_read, (uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb),
        (override));
    MOCK_METHOD(int, uv_fs_readlink, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_stat, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_utime, (uv_loop_t *, uv_fs_t *, const char *, double, double, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_scandir, (uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_scandir_next, (uv_fs_t *, uv_dirent_t *), (override));
    MOCK_METHOD(int, uv_fs_rmdir, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_symlink, (uv_loop_t *, uv_fs_t *, const char *, const char *, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_open, (uv_loop_t *, uv_fs_t *, const char *, int, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_ftruncate, (uv_loop_t *, uv_fs_t *, uv_file, int64_t, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_write, (uv_loop_t *, uv_fs_t *, uv_file, const uv_buf_t[], unsigned int, int64_t, uv_fs_cb),
        (override));
    MOCK_METHOD(int, uv_fs_realpath, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_close, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_fdatasync, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_mkdir, (uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_access, (uv_loop_t *, uv_fs_t *, const char *, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_mkdtemp, (uv_loop_t *, uv_fs_t *, const char *tpl, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_unlink, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_rename, (uv_loop_t *, uv_fs_t *, const char *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_fsync, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_sendfile, (uv_loop_t *, uv_fs_t *, uv_file, uv_file, int64_t, size_t, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_lstat, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_fstat, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(void, uv_fs_req_cleanup, (uv_fs_t *), (override));

public:
    static std::shared_ptr<UvFsMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<UvFsMock> uvfsMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_FS_MOCK_H