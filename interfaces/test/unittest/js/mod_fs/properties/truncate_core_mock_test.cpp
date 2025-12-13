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

#include "truncate_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_fs_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TruncateCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TruncateCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "TruncateCoreMockTest");
    UvFsMock::EnableMock();
}

void TruncateCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TruncateCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TruncateCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_001
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FAILURE when uv_fs_open fails based on path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_001";

    auto [succ, fileInfo] = GenerateFileInfoFromPath("fakePath/TruncateCoreMockTest_DoTruncate_001.txt");
    ASSERT_TRUE(succ);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = TruncateCore::DoTruncate(fileInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_001";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_002
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FAILURE when uv_fs_ftruncate fails based on path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_002";

    auto [succ, fileInfo] = GenerateFileInfoFromPath("fakePath/TruncateCoreMockTest_DoTruncate_002.txt");
    ASSERT_TRUE(succ);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = TruncateCore::DoTruncate(fileInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_002";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_003
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for SUCCESS based on path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_003";

    auto [succ, fileInfo] = GenerateFileInfoFromPath("fakePath/TruncateCoreMockTest_DoTruncate_003.txt");
    ASSERT_TRUE(succ);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(0));

    auto res = TruncateCore::DoTruncate(fileInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_003";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_004
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FAILURE when uv_fs_ftruncate fails based on fd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_004";

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(-EIO));
    auto res = TruncateCore::DoTruncate(fileInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_004";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_005
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for SUCCESS based on fd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_005";

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(0));
    auto res = TruncateCore::DoTruncate(fileInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test