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

#include "symlink_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SymlinkCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SymlinkCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "SymlinkCoreMockTest");
    UvFsMock::EnableMock();
}

void SymlinkCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SymlinkCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SymlinkCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SymlinkCoreMockTest_DoSymlink_001
 * @tc.desc: Test function of SymlinkCore::DoSymlink interface for FAILURE when uv_fs_symlink fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SymlinkCoreMockTest, SymlinkCoreMockTest_DoSymlink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SymlinkCore-begin SymlinkCoreMockTest_DoSymlink_001";

    string target = "fakePath/SymlinkCoreMockTest_DoSymlink_001.txt";
    string linkPath = "fakePath/SymlinkCoreMockTest_DoSymlink_001_link.txt";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_symlink(_, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = SymlinkCore::DoSymlink(target, linkPath);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "SymlinkCore-end SymlinkCoreMockTest_DoSymlink_001";
}

/**
 * @tc.name: SymlinkCoreMockTest_DoSymlink_002
 * @tc.desc: Test function of SymlinkCore::DoSymlink interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SymlinkCoreMockTest, SymlinkCoreMockTest_DoSymlink_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SymlinkCore-begin SymlinkCoreMockTest_DoSymlink_002";

    string target = "fakePath/SymlinkCoreMockTest_DoSymlink_002.txt";
    string linkPath = "fakePath/SymlinkCoreMockTest_DoSymlink_002_link.txt";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_symlink(_, _, _, _, _, _)).WillOnce(Return(0));

    auto res = SymlinkCore::DoSymlink(target, linkPath);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "SymlinkCore-end SymlinkCoreMockTest_DoSymlink_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
