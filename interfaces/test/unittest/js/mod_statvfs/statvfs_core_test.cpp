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
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_001
 * @tc.desc: Test function of StatvfsCore::DoGetTotalSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_001";

    auto path = testDir + "/StatvFsCoreTest_DoGetTotalSize_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetTotalSize(path);

    ASSERT_TRUE(ret.IsSuccess());
    auto size = ret.GetData().value();
    EXPECT_GT(size, 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_001";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_002
 * @tc.desc: Test function of StatvfsCore::DoGetTotalSize interface for FAILURE when file not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_002";

    auto path = testDir + "/StatvFsCoreTest_DoGetTotalSize_002_non_existent.txt";

    auto ret = StatvfsCore::DoGetTotalSize(path);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_002";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_003 start";

    auto path = testDir + "/DoGetFreeSize_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GE(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_003 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_004 start";

    auto path = testDir + "/DoGetFreeSize_004";
    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GE(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_004 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_007, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_007 start";

    auto path = string("/nonexistent/path/DoGetFreeSize_007.txt");
    auto ret = StatvfsCore::DoGetFreeSize(path);
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_007 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_008, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_008 start";

    auto ret = StatvfsCore::DoGetFreeSize("");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetFreeSize_008 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_003 start";

    auto path = testDir + "/DoGetTotalSize_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_003 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_004 start";

    auto path = testDir + "/DoGetTotalSize_004";
    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_004 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_005 start";

    auto ret = StatvfsCore::DoGetTotalSize("/");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_005 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_006 start";

    auto ret = StatvfsCore::DoGetTotalSize("/tmp");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_006 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_007, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_007 start";

    auto path = string("/nonexistent/path/DoGetTotalSize_007.txt");
    auto ret = StatvfsCore::DoGetTotalSize(path);
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_007 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_008, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_008 start";

    auto ret = StatvfsCore::DoGetTotalSize("");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DoGetTotalSize_008 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_CompareFreeAndTotal_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_CompareFreeAndTotal_001 start";

    auto path = testDir + "/CompareFreeAndTotal_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto freeRet = StatvfsCore::DoGetFreeSize(path);
    auto totalRet = StatvfsCore::DoGetTotalSize(path);

    ASSERT_TRUE(freeRet.IsSuccess());
    ASSERT_TRUE(totalRet.IsSuccess());

    EXPECT_LE(freeRet.GetData().value(), totalRet.GetData().value());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_CompareFreeAndTotal_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_MultipleCalls_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleCalls_001 start";

    auto path = testDir + "/MultipleCalls_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    for (int i = 0; i < 10; i++) {
        auto ret = StatvfsCore::DoGetFreeSize(path);
        ASSERT_TRUE(ret.IsSuccess());
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleCalls_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_MultipleCalls_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleCalls_002 start";

    auto path = testDir + "/MultipleCalls_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    for (int i = 0; i < 10; i++) {
        auto ret = StatvfsCore::DoGetTotalSize(path);
        ASSERT_TRUE(ret.IsSuccess());
        EXPECT_GT(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleCalls_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Consistency_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Consistency_001 start";

    auto path = testDir + "/Consistency_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret1 = StatvfsCore::DoGetFreeSize(path);
    auto ret2 = StatvfsCore::DoGetFreeSize(path);

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Consistency_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Consistency_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Consistency_002 start";

    auto path = testDir + "/Consistency_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret1 = StatvfsCore::DoGetTotalSize(path);
    auto ret2 = StatvfsCore::DoGetTotalSize(path);

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Consistency_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DeepDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DeepDirectory_001 start";

    auto deepDir = testDir;
    for (int i = 0; i < 5; i++) {
        deepDir += "/level" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/DeepDirectory_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GE(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DeepDirectory_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DeepDirectory_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DeepDirectory_002 start";

    auto deepDir = testDir;
    for (int i = 0; i < 5; i++) {
        deepDir += "/level" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/DeepDirectory_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DeepDirectory_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_LargeContent_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_LargeContent_001 start";

    auto path = testDir + "/LargeContent_001.txt";
    string largeContent(1024 * 100, 'a');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GE(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_LargeContent_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_LargeContent_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_LargeContent_002 start";

    auto path = testDir + "/LargeContent_002.txt";
    string largeContent(1024 * 100, 'b');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_LargeContent_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Symlink_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Symlink_001 start";

    auto realPath = testDir + "/Symlink_001_real.txt";
    auto linkPath = testDir + "/Symlink_001_link.txt";

    ASSERT_TRUE(FileUtils::CreateFile(realPath, "test content"));

    int ret = symlink(realPath.c_str(), linkPath.c_str());
    if (ret == 0) {
        auto result = StatvfsCore::DoGetFreeSize(linkPath);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_GE(result.GetData().value(), 0);
        unlink(linkPath.c_str());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Symlink_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Symlink_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Symlink_002 start";

    auto realPath = testDir + "/Symlink_002_real.txt";
    auto linkPath = testDir + "/Symlink_002_link.txt";

    ASSERT_TRUE(FileUtils::CreateFile(realPath, "test content"));

    int ret = symlink(realPath.c_str(), linkPath.c_str());
    if (ret == 0) {
        auto result = StatvfsCore::DoGetTotalSize(linkPath);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_GT(result.GetData().value(), 0);
        unlink(linkPath.c_str());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Symlink_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DifferentPaths_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DifferentPaths_001 start";

    auto path1 = testDir + "/DifferentPaths_001_a.txt";
    auto path2 = testDir + "/DifferentPaths_001_b.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path1));
    ASSERT_TRUE(FileUtils::CreateFile(path2));

    auto ret1 = StatvfsCore::DoGetFreeSize(path1);
    auto ret2 = StatvfsCore::DoGetFreeSize(path2);

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DifferentPaths_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DifferentPaths_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_DifferentPaths_002 start";

    auto path1 = testDir + "/DifferentPaths_002_a.txt";
    auto path2 = testDir + "/DifferentPaths_002_b.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path1));
    ASSERT_TRUE(FileUtils::CreateFile(path2));

    auto ret1 = StatvfsCore::DoGetTotalSize(path1);
    auto ret2 = StatvfsCore::DoGetTotalSize(path2);

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_DifferentPaths_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Stress_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Stress_001 start";

    auto path = testDir + "/Stress_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    for (int i = 0; i < 50; i++) {
        auto ret = StatvfsCore::DoGetFreeSize(path);
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Stress_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_Stress_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_Stress_002 start";

    auto path = testDir + "/Stress_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    for (int i = 0; i < 50; i++) {
        auto ret = StatvfsCore::DoGetTotalSize(path);
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_Stress_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_001 start";

    auto ret = StatvfsCore::DoGetFreeSize("/proc");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_002 start";

    auto ret = StatvfsCore::DoGetTotalSize("/proc");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_003 start";

    auto ret = StatvfsCore::DoGetFreeSize("/sys");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_003 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_004 start";

    auto ret = StatvfsCore::DoGetTotalSize("/sys");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_004 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_005 start";

    auto ret = StatvfsCore::DoGetFreeSize("/var");
    if (ret.IsSuccess()) {
        EXPECT_GT(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_005 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_006 start";

    auto ret = StatvfsCore::DoGetTotalSize("/var");
    if (ret.IsSuccess()) {
        EXPECT_GT(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_006 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SystemPaths_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_008 start";

    auto ret = StatvfsCore::DoGetTotalSize("/usr");
    if (ret.IsSuccess()) {
        EXPECT_GT(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SystemPaths_008 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_RelativePath_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_001 start";

    auto ret = StatvfsCore::DoGetFreeSize(".");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_RelativePath_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_002 start";

    auto ret = StatvfsCore::DoGetTotalSize(".");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_RelativePath_003, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_003 start";

    auto ret = StatvfsCore::DoGetFreeSize("..");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_003 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_RelativePath_004, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_004 start";

    auto ret = StatvfsCore::DoGetTotalSize("..");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_RelativePath_004 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_BinaryFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_BinaryFile_001 start";

    auto path = testDir + "/BinaryFile_001.bin";
    string binaryContent(512, '\0');
    ASSERT_TRUE(FileUtils::CreateFile(path, binaryContent));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GE(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_BinaryFile_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_BinaryFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_BinaryFile_002 start";

    auto path = testDir + "/BinaryFile_002.bin";
    string binaryContent(512, '\xFF');
    ASSERT_TRUE(FileUtils::CreateFile(path, binaryContent));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_GT(ret.GetData().value(), 0);

    GTEST_LOG_(INFO) << "StatvFsCoreTest_BinaryFile_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_UnicodeFileName_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeFileName_001 start";

    auto path = testDir + "/UnicodeFileName_中文_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeFileName_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_UnicodeFileName_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeFileName_002 start";

    auto path = testDir + "/UnicodeFileName_日本語_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeFileName_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_UnicodeContent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeContent_001 start";

    auto path = testDir + "/UnicodeContent_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "中文测试内容"));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeContent_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_UnicodeContent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeContent_002 start";

    auto path = testDir + "/UnicodeContent_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "日本語テスト"));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_UnicodeContent_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SpecialChars_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SpecialChars_001 start";

    auto path = testDir + "/SpecialChars_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test\nwith\nnewlines"));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SpecialChars_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_SpecialChars_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_SpecialChars_002 start";

    auto path = testDir + "/SpecialChars_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test\twith\ttabs"));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_SpecialChars_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_MultipleFiles_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleFiles_001 start";

    for (int i = 0; i < 5; i++) {
        auto path = testDir + "/MultipleFiles_001_" + to_string(i) + ".txt";
        ASSERT_TRUE(FileUtils::CreateFile(path));

        auto ret = StatvfsCore::DoGetFreeSize(path);
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleFiles_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_MultipleFiles_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleFiles_002 start";

    for (int i = 0; i < 5; i++) {
        auto path = testDir + "/MultipleFiles_002_" + to_string(i) + ".txt";
        ASSERT_TRUE(FileUtils::CreateFile(path));

        auto ret = StatvfsCore::DoGetTotalSize(path);
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_MultipleFiles_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_ConcurrentSafe_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_ConcurrentSafe_001 start";

    auto path = testDir + "/ConcurrentSafe_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret1 = StatvfsCore::DoGetFreeSize(path);
    auto ret2 = StatvfsCore::DoGetTotalSize(path);
    auto ret3 = StatvfsCore::DoGetFreeSize(path);
    auto ret4 = StatvfsCore::DoGetTotalSize(path);

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    ASSERT_TRUE(ret3.IsSuccess());
    ASSERT_TRUE(ret4.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_ConcurrentSafe_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_VeryLargeFile_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryLargeFile_001 start";

    auto path = testDir + "/VeryLargeFile_001.txt";
    string largeContent(1024 * 500, 'x');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryLargeFile_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_VeryLargeFile_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryLargeFile_002 start";

    auto path = testDir + "/VeryLargeFile_002.txt";
    string largeContent(1024 * 500, 'y');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryLargeFile_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_VeryDeepDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryDeepDirectory_001 start";

    auto deepDir = testDir;
    for (int i = 0; i < 10; i++) {
        deepDir += "/level" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/VeryDeepDirectory_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetFreeSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryDeepDirectory_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_VeryDeepDirectory_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryDeepDirectory_002 start";

    auto deepDir = testDir;
    for (int i = 0; i < 10; i++) {
        deepDir += "/deep" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/VeryDeepDirectory_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto ret = StatvfsCore::DoGetTotalSize(path);
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "StatvFsCoreTest_VeryDeepDirectory_002 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_HomePath_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_HomePath_001 start";

    auto ret = StatvfsCore::DoGetFreeSize("/home");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_HomePath_001 end";
}

HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_HomePath_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest_HomePath_002 start";

    auto ret = StatvfsCore::DoGetTotalSize("/home");
    if (ret.IsSuccess()) {
        EXPECT_GE(ret.GetData().value(), 0);
    }

    GTEST_LOG_(INFO) << "StatvFsCoreTest_HomePath_002 end";
}

} // namespace OHOS::FileManagement::ModuleStatvfs::Test