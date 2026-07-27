/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/statvfs.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleStatfs::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using FileUtils = OHOS::FileManagement::ModuleFileIO::Test::FileUtils;

class StatfsCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/StatfsCoreTest";
};

void StatfsCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "StatfsCoreTest");
}

void StatfsCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void StatfsCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void StatfsCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetBSize_001
 * @tc.desc: Test statvfs f_bsize retrieval with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetBSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetBSize_001";

    auto path = testDir + "/GetBSize_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);
    EXPECT_GT(diskInfo.f_bsize, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetBSize_001";
}

/**
 * @tc.name: GetBAvail_001
 * @tc.desc: Test statvfs f_bavail retrieval with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetBAvail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetBAvail_001";

    auto path = testDir + "/GetBAvail_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);
    EXPECT_GT(diskInfo.f_bavail, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetBAvail_001";
}

/**
 * @tc.name: GetBlocks_001
 * @tc.desc: Test statvfs f_blocks retrieval with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetBlocks_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetBlocks_001";

    auto path = testDir + "/GetBlocks_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);
    EXPECT_GT(diskInfo.f_blocks, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetBlocks_001";
}

/**
 * @tc.name: GetBFree_001
 * @tc.desc: Test statvfs f_bfree retrieval with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetBFree_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetBFree_001";

    auto path = testDir + "/GetBFree_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);
    EXPECT_GT(diskInfo.f_bfree, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetBFree_001";
}

/**
 * @tc.name: InvalidPath_001
 * @tc.desc: Test statvfs with non-existent path for FAILURE across all fields.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, InvalidPath_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin InvalidPath_001";

    auto path = testDir + "/InvalidPath_001_non_existent.txt";

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end InvalidPath_001";
}

/**
 * @tc.name: GetFreeBytes_001
 * @tc.desc: Test free bytes computation (f_bsize * f_bfree) with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetFreeBytes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetFreeBytes_001";

    auto path = testDir + "/GetFreeBytes_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);

    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bfree);
    EXPECT_GT(freeSize, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetFreeBytes_001";
}

/**
 * @tc.name: GetTotalBytes_001
 * @tc.desc: Test total bytes computation (f_bsize * f_blocks) with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetTotalBytes_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetTotalBytes_001";

    auto path = testDir + "/GetTotalBytes_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);

    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                   static_cast<unsigned long long>(diskInfo.f_blocks);
    EXPECT_GT(totalSize, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetTotalBytes_001";
}

/**
 * @tc.name: GetFrSize_001
 * @tc.desc: Test free size computation (f_bsize * f_bavail) with valid path for SUCCESS.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, GetFrSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin GetFrSize_001";

    auto path = testDir + "/GetFrSize_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);

    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bavail);
    EXPECT_GT(freeSize, 0);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end GetFrSize_001";
}

/**
 * @tc.name: StatvfsConsistency_001
 * @tc.desc: Verify free size <= total size and f_bavail <= f_bfree consistency.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(StatfsCoreTest, StatvfsConsistency_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatfsCoreTest-begin StatvfsConsistency_001";

    auto path = testDir + "/StatvfsConsistency_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    ASSERT_EQ(ret, 0);

    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bfree);
    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                   static_cast<unsigned long long>(diskInfo.f_blocks);
    EXPECT_LE(freeSize, totalSize);
    EXPECT_LE(diskInfo.f_bavail, diskInfo.f_bfree);

    GTEST_LOG_(INFO) << "StatfsCoreTest-end StatvfsConsistency_001";
}

} // namespace OHOS::FileManagement::ModuleStatfs::Test
