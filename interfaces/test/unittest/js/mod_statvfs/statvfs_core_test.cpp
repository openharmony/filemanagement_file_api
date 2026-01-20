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

#include "statvfs_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/statvfs.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleStatvfs::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using FileUtils = OHOS::FileManagement::ModuleFileIO::Test::FileUtils;

class StatvFsCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/StatvFsCoreTest";
};

void StatvFsCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "StatvFsCoreTest");
}

void StatvFsCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void StatvFsCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void StatvFsCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetFreeSize_001
 * @tc.desc: Test function of StatvfsCore::DoGetFreeSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetFreeSize_001";

    auto path = testDir + "/StatvFsCoreTest_DoGetFreeSize_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetFreeSize(path);

    ASSERT_TRUE(ret.IsSuccess());
    auto size = ret.GetData().value();
    EXPECT_GT(size, 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetFreeSize_001";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetFreeSize_002
 * @tc.desc: Test function of StatvfsCore::DoGetFreeSize interface for FAILURE when file not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetFreeSize_002";

    auto path = testDir + "/StatvFsCoreTest_DoGetFreeSize_002_non_existent.txt";

    auto ret = StatvfsCore::DoGetFreeSize(path);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetFreeSize_002";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_003
 * @tc.desc: Test function of StatvfsCore::DoGetTotalSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_003";

    auto path = testDir + "/StatvFsCoreTest_DoGetTotalSize_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetTotalSize(path);

    ASSERT_TRUE(ret.IsSuccess());
    auto size = ret.GetData().value();
    EXPECT_GT(size, 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_003";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_004
 * @tc.desc: Test function of StatvfsCore::DoGetTotalSize interface for FAILURE when file not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_004";

    auto path = testDir + "/StatvFsCoreTest_DoGetTotalSize_004_non_existent.txt";

    auto ret = StatvfsCore::DoGetTotalSize(path);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_004";
}

} // namespace OHOS::FileManagement::ModuleStatvfs::Test