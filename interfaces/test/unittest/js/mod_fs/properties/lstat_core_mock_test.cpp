/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string sandboxTestDir = "/data/storage/el2/base/files/LstatCoreMockTest";
};

void LstatCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "LstatCoreMockTest");
    UvFsMock::EnableMock();
}

void LstatCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void LstatCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LstatCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_001
 * @tc.desc: Test function of LstatCore::DoLstat interface for FAILURE with local path when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_001";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("fakePath/LstatCoreMockTest_DoLstat_001.txt");

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_001";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_002
 * @tc.desc: Test function of LstatCore::DoLstat with file URI for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_002";

    auto uri = "file://com.example.statsupporturi" + sandboxTestDir + "/LstatCoreMockTest_DoLstat_002.txt";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat(uri);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_002";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_003
 * @tc.desc: Test function of LstatCore::DoLstat with datashare URI path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_003";

    auto uri = "datashare://com.example.statsupporturi" + sandboxTestDir + "/LstatCoreMockTest_DoLstat_003.txt";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat(uri);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

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

    auto path = sandboxTestDir + "/LstatCoreMockTest_DoLstat_004.txt";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test