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

#ifndef INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H
#define INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H

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
};

class UvfsMock : public IUvfsMock {
public:
    MOCK_METHOD(int, uv_fs_close, (uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb), (override));

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
#endif // INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_PROPERTIES_MOCK_UV_FS_MOCK_H