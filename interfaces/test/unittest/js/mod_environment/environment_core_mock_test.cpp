/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "environment_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "accesstoken_kit_mock.h"
#include "os_account_manager_mock.h"
#include "parameter_mock.h"
#include "securec.h"
#include "tokenid_kit_mock.h"

namespace OHOS::FileManagement::ModuleEnvironment::Test {
using namespace OHOS::Security::AccessToken;
namespace {
const std::string DEFAULT_HOME_PATH = "/storage/Users/currentUser";
const std::string DEFAULT_DESKTOP_PATH = DEFAULT_HOME_PATH + "/Desktop";
const std::string DEFAULT_DOCUMENT_PATH = DEFAULT_HOME_PATH + "/Documents";
const std::string DEFAULT_DOWNLOAD_PATH = DEFAULT_HOME_PATH + "/Download";
const std::string EXTERNAL_STORAGE_PATH = "/storage/External";
const std::string ROOT_USER_DATA_PATH = "/storage/media/0/local";
const std::string STORAGE_DATA_PATH = "/data";
} // namespace

class EnvironmentCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void EnvironmentCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "EnvironmentCoreTest");
    ParameterMock::EnableMock();
    TokenIdKitMock::EnableMock();
    AccessTokenKitMock::EnableMock();
    OsAccountManagerMock::EnableMock();
}

void EnvironmentCoreTest::TearDownTestSuite()
{
    ParameterMock::DisableMock();
    TokenIdKitMock::DisableMock();
    AccessTokenKitMock::DisableMock();
    OsAccountManagerMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void EnvironmentCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void EnvironmentCoreTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetStorageDataDir_001
 * @tc.desc: Test function of DoGetStorageDataDir interface for FAILURE when is not system app.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetStorageDataDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetStorageDataDir_001";

    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(false));

    auto res = DoGetStorageDataDir();

    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 202);
    EXPECT_EQ(err.GetErrMsg(), "The caller is not a system application");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetStorageDataDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetStorageDataDir_002
 * @tc.desc: Test function of DoGetStorageDataDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetStorageDataDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetStorageDataDir_002";

    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(true));

    auto res = DoGetStorageDataDir();

    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, STORAGE_DATA_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetStorageDataDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDataDir_001
 * @tc.desc: Test function of DoGetUserDataDir interface for FAILURE when is not system app.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDataDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDataDir_001";

    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(false));

    auto res = DoGetUserDataDir();

    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 202);
    EXPECT_EQ(err.GetErrMsg(), "The caller is not a system application");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDataDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDataDir_002
 * @tc.desc: Test function of DoGetUserDataDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDataDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDataDir_002";

    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(true));

    auto res = DoGetUserDataDir();

    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, ROOT_USER_DATA_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDataDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDownloadDir_001
 * @tc.desc: Test function of DoGetUserDownloadDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDownloadDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDownloadDir_001";

    auto parameterMock = ParameterMock::GetMock();
    auto accountManagerMock = OsAccountManagerMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*accountManagerMock, GetOsAccountShortName(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>("fakeName"), testing::Return(ERR_OK)));

    auto res = DoGetUserDownloadDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(accountManagerMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, DEFAULT_DOWNLOAD_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDownloadDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDownloadDir_002
 * @tc.desc: Test function of DoGetUserDownloadDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDownloadDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDownloadDir_002";

    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = DoGetUserDownloadDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 801);
    EXPECT_EQ(err.GetErrMsg(), "The device doesn't support this api");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDownloadDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDesktopDir_001
 * @tc.desc: Test function of DoGetUserDesktopDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDesktopDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDesktopDir_001";

    auto parameterMock = ParameterMock::GetMock();
    auto accountManagerMock = OsAccountManagerMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*accountManagerMock, GetOsAccountShortName(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>("fakeName"), testing::Return(ERR_OK)));

    auto res = DoGetUserDesktopDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(accountManagerMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, DEFAULT_DESKTOP_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDesktopDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDesktopDir_002
 * @tc.desc: Test function of DoGetUserDesktopDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDesktopDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDesktopDir_002";

    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = DoGetUserDesktopDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 801);
    EXPECT_EQ(err.GetErrMsg(), "The device doesn't support this api");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDesktopDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDocumentDir_001
 * @tc.desc: Test function of DoGetUserDocumentDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDocumentDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDocumentDir_001";

    auto parameterMock = ParameterMock::GetMock();
    auto accountManagerMock = OsAccountManagerMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*accountManagerMock, GetOsAccountShortName(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>("fakeName"), testing::Return(ERR_OK)));

    auto res = DoGetUserDocumentDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(accountManagerMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, DEFAULT_DOCUMENT_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDocumentDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDocumentDir_002
 * @tc.desc: Test function of DoGetUserDocumentDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDocumentDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDocumentDir_002";

    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = DoGetUserDocumentDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 801);
    EXPECT_EQ(err.GetErrMsg(), "The device doesn't support this api");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDocumentDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_001
 * @tc.desc: Test function of DoGetExternalStorageDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_001";

    auto parameterMock = ParameterMock::GetMock();
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    auto accessTokenKitMock = AccessTokenKitMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*accessTokenKitMock, VerifyAccessToken(testing::_, testing::_))
        .WillOnce(testing::Return(PermissionState::PERMISSION_GRANTED));

    auto res = DoGetExternalStorageDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    testing::Mock::VerifyAndClearExpectations(accessTokenKitMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, EXTERNAL_STORAGE_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_002
 * @tc.desc: Test function of DoGetExternalStorageDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_002";

    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = DoGetExternalStorageDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 801);
    EXPECT_EQ(err.GetErrMsg(), "The device doesn't support this api");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_003
 * @tc.desc: Test function of DoGetExternalStorageDir interface for FAILURE when is not system app.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_003";

    auto parameterMock = ParameterMock::GetMock();
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    auto accessTokenKitMock = AccessTokenKitMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(false));

    auto res = DoGetExternalStorageDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 202);
    EXPECT_EQ(err.GetErrMsg(), "The caller is not a system application");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_003";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_004
 * @tc.desc: Test function of DoGetExternalStorageDir interface for FAILURE when VerifyAccessToken fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_004";

    auto parameterMock = ParameterMock::GetMock();
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    auto accessTokenKitMock = AccessTokenKitMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*accessTokenKitMock, VerifyAccessToken(testing::_, testing::_))
        .WillOnce(testing::Return(PermissionState::PERMISSION_DENIED));

    auto res = DoGetExternalStorageDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    testing::Mock::VerifyAndClearExpectations(accessTokenKitMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 201);
    EXPECT_EQ(err.GetErrMsg(), "Permission verification failed");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_004";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserHomeDir_001
 * @tc.desc: Test function of DoGetUserHomeDir interface for FAILURE when GetParameter fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserHomeDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserHomeDir_001";

    auto parameterMock = ParameterMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto res = DoGetUserHomeDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 801);
    EXPECT_EQ(err.GetErrMsg(), "The device doesn't support this api");

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserHomeDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserHomeDir_002
 * @tc.desc: Test function of DoGetUserHomeDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserHomeDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserHomeDir_002";

    auto parameterMock = ParameterMock::GetMock();
    auto tokenIdKitMock = TokenIdKitMock::GetMock();
    auto accessTokenKitMock = AccessTokenKitMock::GetMock();
    auto accountManagerMock = OsAccountManagerMock::GetMock();
    EXPECT_CALL(*parameterMock, GetParameter(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](const char *, const char *, char *value, uint32_t len) -> int {
            strcpy_s(value, len, "true");
            return 1;
        }));
    EXPECT_CALL(*tokenIdKitMock, IsSystemAppByFullTokenID(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*accessTokenKitMock, VerifyAccessToken(testing::_, testing::_))
        .WillOnce(testing::Return(PermissionState::PERMISSION_GRANTED));
    EXPECT_CALL(*accountManagerMock, GetOsAccountShortName(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<0>("fakeName"), testing::Return(ERR_OK)));

    auto res = DoGetUserHomeDir();

    testing::Mock::VerifyAndClearExpectations(parameterMock.get());
    testing::Mock::VerifyAndClearExpectations(tokenIdKitMock.get());
    testing::Mock::VerifyAndClearExpectations(accessTokenKitMock.get());
    testing::Mock::VerifyAndClearExpectations(accountManagerMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, DEFAULT_HOME_PATH);

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserHomeDir_002";
}

} // namespace OHOS::FileManagement::ModuleEnvironment::Test