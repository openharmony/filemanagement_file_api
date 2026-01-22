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

#include "os_account_manager_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
namespace Test {

thread_local std::shared_ptr<OsAccountManagerMock> OsAccountManagerMock::accountManagerMock = nullptr;
thread_local bool OsAccountManagerMock::mockable = false;

std::shared_ptr<OsAccountManagerMock> OsAccountManagerMock::GetMock()
{
    if (accountManagerMock == nullptr) {
        accountManagerMock = std::make_shared<OsAccountManagerMock>();
    }
    return accountManagerMock;
}

void OsAccountManagerMock::EnableMock()
{
    mockable = true;
}

void OsAccountManagerMock::DisableMock()
{
    accountManagerMock = nullptr;
    mockable = false;
}

bool OsAccountManagerMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS

namespace OHOS::AccountSA {
using OsAccountManagerMock = OHOS::FileManagement::ModuleEnvironment::Test::OsAccountManagerMock;

ErrCode OsAccountManager::GetOsAccountShortName(std::string &shortName)
{
    if (OsAccountManagerMock::IsMockable()) {
        return OsAccountManagerMock::GetMock()->GetOsAccountShortName(shortName);
    }

    static ErrCode (*realGetOsAccountShortName)(std::string &) = []() {
        auto func = (ErrCode(*)(std::string &))dlsym(RTLD_NEXT,
            "_ZN4OHOS9AccountSA16OsAccountManager21GetOsAccountShortNameERNSt3__h12basic_"
            "stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEE");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real OsAccountManager::GetOsAccountShortName: " << dlerror();
        }
        return func;
    }();

    if (!realGetOsAccountShortName) {
        return ERR_INVALID_VALUE;
    }

    return realGetOsAccountShortName(shortName);
}

} // namespace OHOS::AccountSA
