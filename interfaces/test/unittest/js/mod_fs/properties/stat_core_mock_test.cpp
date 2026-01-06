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

#include "stat_core.h"

#include <cstring>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_fs_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class StatCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void StatCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "StatCoreMockTest");
    UvFsMock::EnableMock();
}

void StatCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void StatCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_001
 * @tc.desc: Test function of StatCore::DoStat interface for FAILURE when uv_fs_stat fails based on the path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_001";

    auto [succ, fileinfo] = GenerateFileInfoFromPath("fakePath/StatCoreMockTest_DoStat_001.txt");
    ASSERT_TRUE(succ);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_001";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_002
 * @tc.desc: Test function of StatCore::DoStat interface for SUCCESS based on the path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_002";

    auto [succ, fileinfo] = GenerateFileInfoFromPath("fakePath/StatCoreMockTest_DoStat_002.txt");
    ASSERT_TRUE(succ);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto *stat = res.GetData().value();
    ASSERT_NE(stat, nullptr);
    delete stat;
    stat = nullptr;

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_002";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_003
 * @tc.desc: Test function of StatCore::DoStat interface for FAILURE when uv_fs_stat fails based on the fd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_003";

    FileInfo fileinfo;
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileinfo.path = nullptr;
    fileinfo.isPath = false;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fstat(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_003";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_004
 * @tc.desc: Test function of StatCore::DoStat interface for SUCCESS based on the fd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_004";

    FileInfo fileinfo;
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileinfo.path = nullptr;
    fileinfo.isPath = false;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fstat(_, _, _, _)).WillOnce(Return(0));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto *stat = res.GetData().value();
    ASSERT_NE(stat, nullptr);
    delete stat;
    stat = nullptr;

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_004";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_005
 * @tc.desc: Test function of StatCore::DoStat with URI parameter for FAILURE when uv_fs_stat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_005";

    auto [succ, fileinfo] =
        GenerateFileInfoFromPath("file://com.example.statsupporturi/fakePath/StatCoreMockTest_DoStat_005.txt");
    ASSERT_TRUE(succ);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_005";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_006
 * @tc.desc: Test function of StatCore::DoStat with remote URI path for FAILURE when uv_fs_stat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_006";

    auto [succ, fileinfo] =
        GenerateFileInfoFromPath("datashare://com.example.statsupporturi/fakePath/StatCoreMockTest_DoStat_006.txt");
    ASSERT_TRUE(succ);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test