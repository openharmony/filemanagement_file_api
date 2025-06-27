/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

namespace OHOS::Security::AccessToken {

bool TokenIdKit::IsSystemAppByFullTokenID(uint64_t tokenId)
{
    return OHOS::FileManagement::Backup::BAccessTokenKit::token->IsSystemAppByFullTokenID(tokenId);
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return OHOS::FileManagement::Backup::BAccessTokenKit::token->VerifyAccessToken(tokenID, permissionName);
}
} // namespace OHOS::Security::AccessToken
