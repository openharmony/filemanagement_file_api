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

#include "read_lines_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadLinesCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ReadLinesCoreMockTest";
};

void ReadLinesCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "ReadLinesCoreMockTest");
    UvFsMock::EnableMock();
}

void ReadLinesCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ReadLinesCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void ReadLinesCoreMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_001
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for SUCCESS with encoding option.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_001";

    string path = testDir + "/ReadLinesCoreMockTest_DoReadLines_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content line1\ncontent line2\n"));
    Options option;
    option.encoding = "utf-8";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));

    auto res = ReadLinesCore::DoReadLines(path, option);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_001";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_002
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for SUCCESS without any options.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_002";

    string path = testDir + "/ReadLinesCoreMockTest_DoReadLines_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));

    auto res = ReadLinesCore::DoReadLines(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_002";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_003
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for FAILURE when uv_fs_stat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_003";

    string path = testDir + "/ReadLinesCoreMockTest_DoReadLines_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    Options option;
    option.encoding = "utf-8";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-EIO));

    auto res = ReadLinesCore::DoReadLines(path, option);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test