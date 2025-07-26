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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H

#include <gmock/gmock.h>

#include "ipc_skeleton.h"
#include "os_account_manager.h"

using ErrCode = int;

namespace OHOS::FileManagement::Backup {
class BIPCSkeleton : public RefBase {
public:
    virtual uint32_t GetCallingTokenID() = 0;
    virtual ErrCode GetOsAccountShortName(std::string &shortName) = 0;

public:
    BIPCSkeleton() = default;
    virtual ~BIPCSkeleton() = default;

public:
    static inline std::shared_ptr<BIPCSkeleton> skeleton = nullptr;
};

class IPCSkeletonMock : public BIPCSkeleton {
public:
    MOCK_METHOD(uint32_t, GetCallingTokenID, ());
    MOCK_METHOD(ErrCode, GetOsAccountShortName, (std::string & shortName));
};
} // namespace OHOS::FileManagement::Backup
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_ENVIRONMENT_IPC_SKELETON_MOCK_H