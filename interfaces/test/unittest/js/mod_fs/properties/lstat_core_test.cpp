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

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class LstatCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/LstatCoreTest";
};

void LstatCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "LstatCoreTest");
}

void LstatCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void LstatCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void LstatCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatCoreTest_DoLstat_001
 * @tc.desc: Test function of LstatCore::DoLstat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreTest, LstatCoreTest_DoLstat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreTest-begin LstatCoreTest_DoLstat_001";

    auto nonExistent = testDir + "/non_existent.txt";
    auto res = LstatCore::DoLstat(nonExistent);
    EXPECT_EQ(res.IsSuccess(), false);
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "LstatCoreTest-end LstatCoreTest_DoLstat_001";
}

/**
 * @tc.name: LstatCoreTest_DoLstat_002
 * @tc.desc: Test function of LstatCore::DoLstat interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreTest, LstatCoreTest_DoLstat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreTest-begin LstatCoreTest_DoLstat_002";

    auto path = testDir + "/LstatCoreTest_DoLstat_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    auto res = LstatCore::DoLstat(path);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "LstatCoreTest-end LstatCoreTest_DoLstat_002";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test