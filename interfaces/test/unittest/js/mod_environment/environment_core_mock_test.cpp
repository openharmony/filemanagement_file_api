/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "accesstoken_kit_mock.h"
#include "environment_core.h"
#include "ipc_skeleton_mock.h"
#include "parameter_mock.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class EnvironmentCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<Backup::AccessTokenKitMock> accessToken = nullptr;
    static inline std::shared_ptr<AppFileService::ParamMoc> paramMoc = nullptr;
    static inline shared_ptr<Backup::IPCSkeletonMock> skeleton = nullptr;
};

void EnvironmentCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    accessToken = std::make_shared<Backup::AccessTokenKitMock>();
    Backup::BAccessTokenKit::token = accessToken;
    paramMoc = std::make_shared<AppFileService::ParamMoc>();
    AppFileService::ParamMoc::paramMoc = paramMoc;
    skeleton = make_shared<Backup::IPCSkeletonMock>();
    Backup::IPCSkeletonMock::skeleton = skeleton;
}

void EnvironmentCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Backup::BAccessTokenKit::token = nullptr;
    accessToken = nullptr;
    AppFileService::IParamMoc::paramMoc = nullptr;
    paramMoc = nullptr;
    Backup::IPCSkeletonMock::skeleton = nullptr;
    skeleton = nullptr;
}

void EnvironmentCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void EnvironmentCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetStorageDataDir_001
 * @tc.desc: Test function of DoGetStorageDataDir interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetStorageDataDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetStorageDataDir_001";

    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(false));

    auto res = ModuleEnvironment::DoGetStorageDataDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetStorageDataDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetStorageDataDir_002
 * @tc.desc: Test function of DoGetStorageDataDir interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetStorageDataDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetStorageDataDir_002";

    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(true));

    auto res = ModuleEnvironment::DoGetStorageDataDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetStorageDataDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDataDir_001
 * @tc.desc: Test function of DoGetUserDataDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDataDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDataDir_001";

    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(false));

    auto res = ModuleEnvironment::DoGetUserDataDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDataDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDataDir_002
 * @tc.desc: Test function of DoGetUserDataDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDataDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDataDir_002";

    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(true));

    auto res = ModuleEnvironment::DoGetUserDataDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDataDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDownloadDir_001
 * @tc.desc: Test function of DoGetUserDownloadDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDownloadDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDownloadDir_001";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillRepeatedly(Invoke([&](const char *, const char *, char *value, uint32_t) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*skeleton, GetOsAccountShortName(_)).WillRepeatedly(Return(ERR_OK));

    auto res = ModuleEnvironment::DoGetUserDownloadDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDownloadDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDownloadDir_002
 * @tc.desc: Test function of DoGetUserDownloadDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDownloadDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDownloadDir_002";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _)).WillOnce(Return(-1));

    auto res = ModuleEnvironment::DoGetUserDownloadDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDownloadDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDownloadDir_003
 * @tc.desc: Test function of DoGetUserDownloadDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDownloadDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDownloadDir_003";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillRepeatedly(Invoke([&](const char *, const char *, char *value, uint32_t) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*skeleton, GetOsAccountShortName(_)).WillRepeatedly(Return(1));

    auto res = ModuleEnvironment::DoGetUserDownloadDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDownloadDir_003";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDesktopDir_001
 * @tc.desc: Test function of DoGetUserDesktopDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDesktopDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDesktopDir_001";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));

    auto res = ModuleEnvironment::DoGetUserDesktopDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDesktopDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDesktopDir_002
 * @tc.desc: Test function of DoGetUserDesktopDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDesktopDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDesktopDir_002";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return -1;
        }));

    auto res = ModuleEnvironment::DoGetUserDesktopDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDesktopDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDesktopDir_003
 * @tc.desc: Test function of DoGetUserDesktopDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDesktopDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDesktopDir_003";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *, const char *, char *value, uint32_t) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*skeleton, GetOsAccountShortName(_)).WillRepeatedly(Return(ERR_OK));

    auto res = ModuleEnvironment::DoGetUserDesktopDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDesktopDir_003";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDocumentDir_001
 * @tc.desc: Test function of DoGetUserDocumentDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDocumentDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDocumentDir_001";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));

    auto res = ModuleEnvironment::DoGetUserDocumentDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDocumentDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDocumentDir_002
 * @tc.desc: Test function of DoGetUserDocumentDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDocumentDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDocumentDir_002";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return -1;
        }));

    auto res = ModuleEnvironment::DoGetUserDocumentDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDocumentDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserDocumentDir_003
 * @tc.desc: Test function of DoGetUserDocumentDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserDocumentDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserDocumentDir_003";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *, const char *, char *value, uint32_t) {
            strcpy_s(value, 5, "true");
            return 1;
        }));

    auto res = ModuleEnvironment::DoGetUserDocumentDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserDocumentDir_003";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_001
 * @tc.desc: Test function of DoGetExternalStorageDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_001";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(false));

    auto res = ModuleEnvironment::DoGetExternalStorageDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_002
 * @tc.desc: Test function of DoGetExternalStorageDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_002";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(true));
    EXPECT_CALL(*skeleton, GetCallingTokenID()).WillOnce(Return(0));
    EXPECT_CALL(*accessToken, VerifyAccessToken(_, _)).WillOnce(Return(1));

    auto res = ModuleEnvironment::DoGetExternalStorageDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_002";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_003
 * @tc.desc: Test function of DoGetExternalStorageDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_003";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(true));
    EXPECT_CALL(*skeleton, GetCallingTokenID()).WillOnce(Return(0));
    EXPECT_CALL(*accessToken, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));

    auto res = ModuleEnvironment::DoGetExternalStorageDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_003";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetExternalStorageDir_004
 * @tc.desc: Test function of DoGetExternalStorageDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetExternalStorageDir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetExternalStorageDir_004";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(false));

    auto res = ModuleEnvironment::DoGetExternalStorageDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetExternalStorageDir_004";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserHomeDir_001
 * @tc.desc: Test function of DoGetUserHomeDir interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserHomeDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserHomeDir_001";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _)).WillOnce(Return(1));

    auto res = ModuleEnvironment::DoGetUserHomeDir();

    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserHomeDir_001";
}

/**
 * @tc.name: EnvironmentCoreTest_DoGetUserHomeDir_002
 * @tc.desc: Test function of DoGetUserHomeDir interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(EnvironmentCoreTest, EnvironmentCoreTest_DoGetUserHomeDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnvironmentCoreTest-begin EnvironmentCoreTest_DoGetUserHomeDir_002";

    EXPECT_CALL(*paramMoc, GetParameter(_, _, _, _))
        .WillOnce(Invoke([&](const char *key, const char *def, char *value, uint32_t len) {
            strcpy_s(value, 5, "true");
            return 1;
        }));
    EXPECT_CALL(*accessToken, IsSystemAppByFullTokenID(_)).WillOnce(Return(true));
    EXPECT_CALL(*skeleton, GetCallingTokenID()).WillOnce(Return(0));
    EXPECT_CALL(*accessToken, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));

    auto res = ModuleEnvironment::DoGetUserHomeDir();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "EnvironmentCoreTest-end EnvironmentCoreTest_DoGetUserHomeDir_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test