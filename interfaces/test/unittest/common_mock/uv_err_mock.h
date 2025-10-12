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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_ERR_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_ERR_MOCK_H

#include "uv.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IUvErr {
public:
    virtual ~IUvErr() = default;
    virtual const char *uv_err_name(int err) = 0;
};

class UvErrMock : public IUvErr {
public:
    MOCK_METHOD(const char *, uv_err_name, (int err), (override));

public:
    static std::shared_ptr<UvErrMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<UvErrMock> uvErrMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_UV_ERR_MOCK_H