/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>

#include "accesstoken_kit.h"
#include "tokenid_kit.h"

namespace OHOS::FileManagement::Backup {
class BAccessTokenKit {
public:
    virtual bool IsSystemAppByFullTokenID(uint64_t) = 0;
    virtual int VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::string &) = 0;

public:
    BAccessTokenKit() = default;
    virtual ~BAccessTokenKit() = default;

public:
    static inline std::shared_ptr<BAccessTokenKit> token = nullptr;
};

class AccessTokenKitMock : public BAccessTokenKit {
public:
    MOCK_METHOD(bool, IsSystemAppByFullTokenID, (uint64_t));
    MOCK_METHOD(int, VerifyAccessToken, (Security::AccessToken::AccessTokenID, const std::string &));
};
} // namespace OHOS::FileManagement::Backup
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_MOCK_ACCESSTOKEN_KIT_MOCK_H