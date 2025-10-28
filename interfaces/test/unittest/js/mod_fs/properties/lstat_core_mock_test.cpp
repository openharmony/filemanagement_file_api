/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "lstat_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class LstatCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LstatCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "LstatCoreMockTest");
    UvFsMock::EnableMock();
}

void LstatCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void LstatCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LstatCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_001
 * @tc.desc: Test function of LstatCore::DoLstat interface for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_001";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("/data/test/lstat.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_001";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_002
 * @tc.desc: Test function of LstatCore::DoLstat with URI for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_002";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("file://com.example.statsupporturi/data/storage/el2/base/files/test.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_002";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_003
 * @tc.desc: Test function of LstatCore::DoLstat with remote URI path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_003";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("datashare://com.example.statsupporturi/data/storage/el2/base/files/test.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_003";
}
/**
 * @tc.name: LstatCoreMockTest_DoLstat_004
 * @tc.desc: Test function of LstatCore::DoLstat with sandbox path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_004";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("/data/storage/el2/base/files/test.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_004";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_005
 * @tc.desc: Test function of LstatCore::DoLstat with docs URI path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_005";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("file://docs/storage/Users/currentUser/Download/DocumentViewPicker1.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_005";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_006
 * @tc.desc: Test function of LstatCore::DoLstat with media URI path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_006";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("file://media/Photo/1/IMG_1609518878_000/IMG_202112_003259.jpg");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test