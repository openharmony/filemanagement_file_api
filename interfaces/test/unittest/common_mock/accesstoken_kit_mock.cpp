/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "accesstoken_kit_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
namespace Test {

thread_local std::shared_ptr<AccessTokenKitMock> AccessTokenKitMock::accessTokenKitMock = nullptr;
thread_local bool AccessTokenKitMock::mockable = false;

std::shared_ptr<AccessTokenKitMock> AccessTokenKitMock::GetMock()
{
    if (accessTokenKitMock == nullptr) {
        accessTokenKitMock = std::make_shared<AccessTokenKitMock>();
    }
    return accessTokenKitMock;
}

void AccessTokenKitMock::EnableMock()
{
    mockable = true;
}

void AccessTokenKitMock::DisableMock()
{
    accessTokenKitMock = nullptr;
    mockable = false;
}

bool AccessTokenKitMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS

namespace OHOS::Security::AccessToken {
using AccessTokenKitMock = OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock;

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    if (AccessTokenKitMock::IsMockable()) {
        return AccessTokenKitMock::GetMock()->VerifyAccessToken(tokenID, permissionName);
    }

    static int (*realVerifyAccessToken)(AccessTokenID, const std::string &) = []() {
        auto func = (int (*)(AccessTokenID, const std::string &))dlsym(RTLD_NEXT,
            "_ZN4OHOS8Security11AccessToken14AccessTokenKit17VerifyAccessTokenEjRKNSt3__h12basic_"
            "stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real AccessTokenKit::VerifyAccessToken: " << dlerror();
        }
        return func;
    }();

    if (!realVerifyAccessToken) {
        return -1;
    }

    return realVerifyAccessToken(tokenID, permissionName);
}

} // namespace OHOS::Security::AccessToken
