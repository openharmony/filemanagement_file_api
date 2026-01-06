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

#include "fdatasync_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FDataSyncCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FDataSyncCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FDataSyncCoreMockTest");
    UvFsMock::EnableMock();
}

void FDataSyncCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FDataSyncCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FDataSyncCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FDataSyncCoreMockTest_DoFDataSync_001
 * @tc.desc: Test function of FDataSyncCore::DoFDataSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FDataSyncCoreMockTest, FDataSyncCoreMockTest_DoFDataSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FDataSyncCoreMockTest-begin FDataSyncCoreMockTest_DoFDataSync_001";

    int fd = 1;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fdatasync(_, _, _, _)).WillOnce(Return(0));

    auto res = FDataSyncCore::DoFDataSync(fd);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "FDataSyncCoreMockTest-end FDataSyncCoreMockTest_DoFDataSync_001";
}

/**
 * @tc.name: FDataSyncCoreMockTest_DoFDataSync_002
 * @tc.desc: Test function of FDataSyncCore::DoFDataSync interface for FAILURE when uv_fs_fdatasync fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FDataSyncCoreMockTest, FDataSyncCoreMockTest_DoFDataSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FDataSyncCoreMockTest-begin FDataSyncCoreMockTest_DoFDataSync_002";

    int fd = 1;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fdatasync(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = FDataSyncCore::DoFDataSync(fd);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FDataSyncCoreMockTest-end FDataSyncCoreMockTest_DoFDataSync_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test