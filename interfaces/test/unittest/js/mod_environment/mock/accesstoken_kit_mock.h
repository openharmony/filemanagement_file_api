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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_MOCK_ACCESSTOKEN_KIT_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_MOCK_ACCESSTOKEN_KIT_MOCK_H

#include "accesstoken_kit.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleEnvironment::Test {
using AccessTokenID = OHOS::Security::AccessToken::AccessTokenID;

class IAccessTokenKitMock {
public:
    virtual ~IAccessTokenKitMock() = default;
    virtual int VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName) = 0;
};

class AccessTokenKitMock : public IAccessTokenKitMock {
public:
    MOCK_METHOD(int, VerifyAccessToken, (AccessTokenID, const std::string &));

public:
    static std::shared_ptr<AccessTokenKitMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<AccessTokenKitMock> accessTokenKitMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleEnvironment::Test
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_MOCK_ACCESSTOKEN_KIT_MOCK_H