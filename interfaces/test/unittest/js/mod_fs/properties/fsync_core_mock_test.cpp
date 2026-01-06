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

#include "fsync_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsyncCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsyncCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsyncCoreMockTest");
    UvFsMock::EnableMock();
}

void FsyncCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsyncCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsyncCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsyncCoreMockTest_DoFsync_001
 * @tc.desc: Test function of RenameCore::DoFsync interface for FAILURE when uv_fs_fsync fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncCoreMockTest, FsyncCoreMockTest_DoFsync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncCoreMockTest-begin FsyncCoreMockTest_DoFsync_001";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fsync(_, _, _, _)).WillOnce(Return(-1));

    auto res = FsyncCore::DoFsync(1);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsyncCoreMockTest-end FsyncCoreMockTest_DoFsync_001";
}

/**
 * @tc.name: FsyncCoreMockTest_DoFsync_002
 * @tc.desc: Test function of RenameCore::DoFsync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncCoreMockTest, FsyncCoreMockTest_DoFsync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncCoreMockTest-begin FsyncCoreMockTest_DoFsync_002";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fsync(_, _, _, _)).WillOnce(Return(0));

    auto res = FsyncCore::DoFsync(1);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "FsyncCoreMockTest-end FsyncCoreMockTest_DoFsync_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test