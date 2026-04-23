/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "environment_n_exporter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "accesstoken_kit_mock.h"
#include "libn_mock.h"
#include "os_account_manager_mock.h"
#include "parameter_mock.h"
#include "securec.h"
#include "tokenid_kit_mock.h"

namespace OHOS::FileManagement::ModuleEnvironment::Test {
using namespace OHOS::Security::AccessToken;

class EnvironmentNExporterMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void EnvironmentNExporterMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "EnvironmentNExporterMockTest");
    OHOS::FileManagement::ModuleFileIO::Test::LibnMock::EnableMock();
    ParameterMock::EnableMock();
    TokenIdKitMock::EnableMock();
    AccessTokenKitMock::EnableMock();
    OsAccountManagerMock::EnableMock();
}

void EnvironmentNExporterMockTest::TearDownTestSuite()
{
    OHOS::FileManagement::ModuleFileIO::Test::LibnMock::DisableMock();
    ParameterMock::DisableMock();
    TokenIdKitMock::DisableMock();
    AccessTokenKitMock::DisableMock();
    OsAccountManagerMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void EnvironmentNExporterMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void EnvironmentNExporterMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: EnvironmentNExporterMockTest_GetStorageDataDir_001
 * @tc.desc: Test function of GetStorageDataDir interface for FAILURE when is not system app.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentNExporterMockTest, EnvironmentNExporterMockTest_GetStorageDataDir_001,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-begin EnvironmentNExporterMockTest_GetStorageDataDir_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(false));

    auto res = GetStorageDataDir(env, info);

    EXPECT_EQ(res, nullptr);
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());

    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-end EnvironmentNExporterMockTest_GetStorageDataDir_001";
}

/**
 * @tc.name: EnvironmentNExporterMockTest_GetUserDataDir_001
 * @tc.desc: Test function of GetUserDataDir interface for FAILURE when is not system app.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentNExporterMockTest, EnvironmentNExporterMockTest_GetUserDataDir_001,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-begin EnvironmentNExporterMockTest_GetUserDataDir_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(false));

    auto res = GetUserDataDir(env, info);

    EXPECT_EQ(res, nullptr);
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());

    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-end EnvironmentNExporterMockTest_GetUserDataDir_001";
}

/**
 * @tc.name: EnvironmentNExporterMockTest_GetUserDownloadDir_001
 * @tc.desc: Test function of GetUserDownloadDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentNExporterMockTest, EnvironmentNExporterMockTest_GetUserDownloadDir_001,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-begin EnvironmentNExporterMockTest_GetUserDownloadDir_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = GetUserDownloadDir(env, info);

    EXPECT_EQ(res, nullptr);
    testing::Mock::VerifyAndClearExpectations(parameterMock.get());

    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-end EnvironmentNExporterMockTest_GetUserDownloadDir_001";
}

/**
 * @tc.name: EnvironmentNExporterMockTest_GetUserDesktopDir_001
 * @tc.desc: Test function of GetUserDesktopDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentNExporterMockTest, EnvironmentNExporterMockTest_GetUserDesktopDir_001,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-begin EnvironmentNExporterMockTest_GetUserDesktopDir_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = GetUserDesktopDir(env, info);

    EXPECT_EQ(res, nullptr);
    testing::Mock::VerifyAndClearExpectations(parameterMock.get());

    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-end EnvironmentNExporterMockTest_GetUserDesktopDir_001";
}

/**
 * @tc.name: EnvironmentNExporterMockTest_GetUserDocumentDir_001
 * @tc.desc: Test function of GetUserDocumentDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentNExporterMockTest, EnvironmentNExporterMockTest_GetUserDocumentDir_001,
         testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-begin EnvironmentNExporterMockTest_GetUserDocumentDir_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = GetUserDocumentDir(env, info);

    EXPECT_EQ(res, nullptr);
    testing::Mock::VerifyAndClearExpectations(parameterMock.get());

    GTEST_LOG_(INFO) << "EnvironmentNExporterMockTest-end EnvironmentNExporterMockTest_GetUserDocumentDir_001";
}

} // namespace OHOS::FileManagement::ModuleEnvironment::Test
