/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H

#include "os_account_manager.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleEnvironment::Test {

class IOsAccountManagerMock {
public:
    virtual ~IOsAccountManagerMock() = default;
    virtual ErrCode GetOsAccountShortName(std::string &shortName) = 0;
};

class OsAccountManagerMock : public IOsAccountManagerMock {
public:
    MOCK_METHOD(ErrCode, GetOsAccountShortName, (std::string &), (override));

public:
    static std::shared_ptr<OsAccountManagerMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<OsAccountManagerMock> accountManagerMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleEnvironment::Test
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H