/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "tokenid_kit_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
namespace Test {

thread_local std::shared_ptr<TokenIdKitMock> TokenIdKitMock::tokenIdKitMock = nullptr;
thread_local bool TokenIdKitMock::mockable = false;

std::shared_ptr<TokenIdKitMock> TokenIdKitMock::GetMock()
{
    if (tokenIdKitMock == nullptr) {
        tokenIdKitMock = std::make_shared<TokenIdKitMock>();
    }
    return tokenIdKitMock;
}

void TokenIdKitMock::EnableMock()
{
    mockable = true;
}

void TokenIdKitMock::DisableMock()
{
    tokenIdKitMock = nullptr;
    mockable = false;
}

bool TokenIdKitMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS

namespace OHOS::Security::AccessToken {
using TokenIdKitMock = OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock;

bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t tokenId)
{
    if (TokenIdKitMock::IsMockable()) {
        return TokenIdKitMock::GetMock()->IsSystemAppByFullTokenID(tokenId);
    }

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__LP64__)
    static constexpr const char *realIsSystemAppByFullTokenIDSymbol =
        "_ZN4OHOS8Security11AccessToken10TokenIdKit24IsSystemAppByFullTokenIDEm";
#else
    static constexpr const char *realIsSystemAppByFullTokenIDSymbol =
        "_ZN4OHOS8Security11AccessToken10TokenIdKit24IsSystemAppByFullTokenIDEy";
#endif

    static bool (*realIsSystemAppByFullTokenID)(uint64_t) = []() {
        auto func = (bool (*)(uint64_t))dlsym(RTLD_NEXT, realIsSystemAppByFullTokenIDSymbol);
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real TokenIdKit::IsSystemAppByFullTokenID: " << dlerror();
        }
        return func;
    }();

    if (!realIsSystemAppByFullTokenID) {
        return false;
    }

    return realIsSystemAppByFullTokenID(tokenId);
}

} // namespace OHOS::Security::AccessToken
