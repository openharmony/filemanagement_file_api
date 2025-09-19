/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_UV_FS_MOCK_H
#define INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_UV_FS_MOCK_H

#include "uv.h"
#include "filemgmt_libn.h"

#include <cstdio>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;

class IUvfsMock {
public:
    virtual ~IUvfsMock() = default;
    virtual int uv_fs_close(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb) = 0;
    virtual int uv_fs_mkdtemp(uv_loop_t *loop, uv_fs_t *req, const char *tpl, uv_fs_cb cb) = 0;
    virtual int uv_fs_rename(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, uv_fs_cb cb) = 0;
    virtual int uv_fs_open(uv_loop_t *loop, uv_fs_t *req, const char *path, int flags, int mode, uv_fs_cb cb) = 0;
    virtual int uv_fs_ftruncate(uv_loop_t *loop, uv_fs_t *req, uv_file fd, int64_t offset, uv_fs_cb cb) = 0;
    virtual void uv_fs_req_cleanup(uv_fs_t* req) = 0;
    virtual int uv_fs_fdatasync(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb) = 0;
    virtual int uv_fs_unlink(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_stat(uv_loop_t *loop, uv_fs_t *req, const char *path, uv_fs_cb cb) = 0;
    virtual int uv_fs_utime(uv_loop_t *loop, uv_fs_t *req, const char *path, double atime,
        double mtime, uv_fs_cb cb) = 0;
    virtual int uv_fs_symlink(uv_loop_t *loop, uv_fs_t *req, const char *path, const char *newPath, int flags,
        uv_fs_cb cb) = 0;
};

class UvfsMock : public IUvfsMock {
public:
    MOCK_METHOD(int, uv_fs_close, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_mkdtemp, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_rename, (uv_loop_t *, uv_fs_t *, const char *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_open, (uv_loop_t *, uv_fs_t *, const char *, int, int, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_ftruncate, (uv_loop_t *, uv_fs_t *, uv_file, int64_t, uv_fs_cb), (override));
    MOCK_METHOD(void, uv_fs_req_cleanup, (uv_fs_t *), (override));
    MOCK_METHOD(int, uv_fs_fdatasync, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_unlink, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_stat, (uv_loop_t *, uv_fs_t *, const char *, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_utime, (uv_loop_t *, uv_fs_t *, const char *, double, double, uv_fs_cb), (override));
    MOCK_METHOD(int, uv_fs_symlink, (uv_loop_t *, uv_fs_t *, const char *, const char *, int, uv_fs_cb), (override));

public:
    static std::shared_ptr<UvfsMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<UvfsMock> uvfsMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_UV_FS_MOCK_H