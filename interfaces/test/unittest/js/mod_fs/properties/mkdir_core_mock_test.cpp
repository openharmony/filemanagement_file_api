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

#include "mkdir_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MkdirCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/MkdirCoreMockTest";
};

void MkdirCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "MkdirCoreMockTest");
    UvFsMock::EnableMock();
}

void MkdirCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void MkdirCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void MkdirCoreMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_001
 * @tc.desc: Test function of MkdirCore::DoMkdir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_001";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_mkdir(_, _, _, _, _)).WillOnce(Return(0));

    string path = testDir + "/MkdirCoreMockTest_DoMkdir_001";
    auto ret = MkdirCore::DoMkdir(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_001";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_002
 * @tc.desc: Test function of MkdirCore::DoMkdir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_002";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).Times(2).WillOnce(Return(-ENOENT)).WillOnce(Return(ERRNO_NOERR));

    string path = testDir + "/MkdirCoreMockTest_DoMkdir_002";
    auto ret = MkdirCore::DoMkdir(path, true);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_002";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_003
 * @tc.desc: Test function of MkdirCore::DoMkdir interface for FAILURE when uv_fs_mkdir fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_003";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_mkdir(_, _, _, _, _)).WillOnce(Return(EEXIST));

    string path = testDir + "/MkdirCoreMockTest_DoMkdir_003";
    auto ret = MkdirCore::DoMkdir(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(ret.IsSuccess(), false);
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900015);
    EXPECT_EQ(err.GetErrMsg(), "File exists");

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_003";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_004
 * @tc.desc: Test function of MkdirCore::DoMkdir interface for FAILURE when file exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_004";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(ERRNO_NOERR));

    string path = testDir + "/MkdirCoreMockTest_DoMkdir_004";
    auto ret = MkdirCore::DoMkdir(path, true);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(ret.IsSuccess(), false);
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900015);
    EXPECT_EQ(err.GetErrMsg(), "File exists");

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_004";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_005
 * @tc.desc: Test function of MkdirCore::DoMkdir interface for FAILURE when I/O error.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_005";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(EIO));

    string path = testDir + "/MkdirCoreMockTest_DoMkdir_005";
    auto ret = MkdirCore::DoMkdir(path, true);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(ret.IsSuccess(), false);
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test