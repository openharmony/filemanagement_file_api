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
 
#ifndef INTERFACES_TEST_UNITTEST_FDATASYNC_MOCK_H
#define INTERFACES_TEST_UNITTEST_FDATASYNC_MOCK_H

#include "filemgmt_libn.h"
#include "fdatasync.h"

#include <cstdio>
#include <filesystem>
#include <gmock/gmock.h>
#include <tuple>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;
using std::tuple;

class IFdatasyncMock {
public:
    virtual ~IFdatasyncMock() = default;
    virtual NVal CreateUndefined(napi_env env) = 0;
};

class FdatasyncMock : public IFdatasyncMock {
public:
MOCK_METHOD(NVal, CreateUndefined, (napi_env), (override));

public:
    static std::shared_ptr<FdatasyncMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<FdatasyncMock> fdatasyncMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_FDATASYNC_MOCK_H