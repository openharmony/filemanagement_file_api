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

#include "copy_dir_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyDirCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CopyDirCoreTest";
    const string srcDir = testDir + "/srcDir";
    const string destDir = testDir + "/destDir";
};

void CopyDirCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CopyDirCoreTest");
}

void CopyDirCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyDirCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(srcDir));
    ASSERT_TRUE(FileUtils::CreateDirectories(destDir));
}

void CopyDirCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_001
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for SUCCESS with empty directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_001";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_001";
    string dest = destDir;
    ASSERT_TRUE(FileUtils::CreateDirectories(src));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_001";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_002
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for FAILURE with invalid mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_002";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_002";
    string dest = destDir;
    ASSERT_TRUE(FileUtils::CreateDirectories(src));

    int invalidMode = COPYMODE_MAX + 1;
    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(invalidMode));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_002";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_003
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for FAILURE with non-existent source.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_003";

    string src = srcDir + "/non_existent_dir";
    string dest = destDir;

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_003";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_004
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for FAILURE when dest path is a file, not a directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_004";

    string src = srcDir;
    string dest = destDir + "/CopyDirCoreTest_DoCopyDir_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(dest, "content"));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_004";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_005
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for FAILURE with same source and destination.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_005";

    string src = srcDir;
    string dest = srcDir;

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_005";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_006
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for SUCCESS with files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_006";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_006";
    string file1 = src + "/file1.txt";
    string file2 = src + "/file2.txt";
    string dest = destDir;

    ASSERT_TRUE(FileUtils::CreateFile(file1, "content1"));
    ASSERT_TRUE(FileUtils::CreateFile(file2, "content2"));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_006";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_007
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for SUCCESS with subdirectories.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_007";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_007";
    string subDir1 = src + "/subDir1";
    string subDir2 = src + "/subDir2";
    string file1 = subDir1 + "/file1.txt";
    string file2 = subDir2 + "/file2.txt";
    string dest = destDir;
    ASSERT_TRUE(FileUtils::CreateFile(file1, "sub1_content1"));
    ASSERT_TRUE(FileUtils::CreateFile(file2, "sub2_content2"));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_007";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_008
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for FAILURE with existing files (throw error mode).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_008";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_008";
    string srcFile = src + "/file.txt";
    string dest = destDir;
    ASSERT_TRUE(FileUtils::CreateFile(srcFile, "content"));

    string destFile = dest + "/CopyDirCoreTest_DoCopyDir_008/file.txt";
    ASSERT_TRUE(FileUtils::CreateFile(destFile, "exists_file_content"));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(DIRMODE_FILE_COPY_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_008";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_009
 * @tc.desc: Test function of CopyDirCore::DoCopyDir interface for SUCCESS with existing files (overwrite mode).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_009";

    string src = srcDir + "/CopyDirCoreTest_DoCopyDir_009";
    string srcFile = src + "/file.txt";
    string dest = destDir;
    ASSERT_TRUE(FileUtils::CreateFile(srcFile, "content"));

    string destFile = dest + "/CopyDirCoreTest_DoCopyDir_009/file.txt";
    ASSERT_TRUE(FileUtils::CreateFile(destFile, "exists_file_content"));

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(DIRMODE_FILE_COPY_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    auto [succ, content] = FileUtils::ReadTextFileContent(destFile);
    EXPECT_TRUE(succ);
    EXPECT_EQ(content, "content");

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_009";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test