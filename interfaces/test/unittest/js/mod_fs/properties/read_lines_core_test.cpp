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

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadLinesCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ReadLinesCoreTest";
};

void ReadLinesCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ReadLinesCoreTest");
}

void ReadLinesCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ReadLinesCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void ReadLinesCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadLinesCoreTest_DoReadLines_001
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for FAILURE when path is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreTest, ReadLinesCoreTest_DoReadLines_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreTest-begin ReadLinesCoreTest_DoReadLines_001";

    string path = testDir + "/ReadLinesCoreTest_DoReadLines_001_non_existent.txt";
    Options option;
    option.encoding = "utf-8";

    auto res = ReadLinesCore::DoReadLines(path, option);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "ReadLinesCoreTest-end ReadLinesCoreTest_DoReadLines_001";
}

/**
 * @tc.name: ReadLinesCoreTest_DoReadLines_002
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for FAILURE when encoding is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreTest, ReadLinesCoreTest_DoReadLines_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreTest-begin ReadLinesCoreTest_DoReadLines_002";

    string path = testDir + "/ReadLinesCoreTest_DoReadLines_002.txt";
    Options option;
    option.encoding = "utf-16";

    auto res = ReadLinesCore::DoReadLines(path, option);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadLinesCoreTest-end ReadLinesCoreTest_DoReadLines_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test