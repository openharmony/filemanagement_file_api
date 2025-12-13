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

#include "movedir_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveDirCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/MoveDirCoreTest";
};

void MoveDirCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "MoveDirCoreTest");
}

void MoveDirCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void MoveDirCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void MoveDirCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_001
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_001";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_001/src/testDir001";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_001/dest";
    auto expectedDestDir = dest + "/testDir001";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());
    EXPECT_TRUE(FileUtils::Exists(expectedDestDir));

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_001";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_002
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_002";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_002/src/testDir002";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_002/dest";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    int invalidMode = DIRMODE_MAX + 1;
    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(invalidMode));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_002";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_003
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when src path is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_003";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_003/src/testDir003";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_003/dest";

    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_003";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_004
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when dest path is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_004";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_004/src/testDir004";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_004/dest";

    ASSERT_TRUE(FileUtils::CreateDirectories(src));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_004";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_005
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when dest directory is not empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_005";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_005/src/testDir005";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_005/dest";
    auto destSubDir = dest + "/testDir005/subDir";

    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateDirectories(destSubDir));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900032);
    EXPECT_EQ(err.GetErrMsg(), "Directory not empty");
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_005";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_006
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_006";

    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_006/src/testDir006";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_006/dest";
    auto subDir = src + "/testSubDir";
    auto subFile = src + "/testSubFile.txt";
    auto expectedDestDir = dest + "/testDir006";
    auto expectedDestSubDir = dest + "/testDir006/testSubDir";
    auto expectedDestSubFile = dest + "/testDir006/testSubFile.txt";

    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    ASSERT_TRUE(FileUtils::CreateFile(subFile, "content"));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_TRUE(FileUtils::Exists(expectedDestDir));
    EXPECT_TRUE(FileUtils::Exists(expectedDestSubDir));
    EXPECT_TRUE(FileUtils::Exists(expectedDestSubFile));
    EXPECT_EQ(FileUtils::ReadTextFileContent(expectedDestSubFile), std::make_tuple(true, "content"));

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_006";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_007
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when exiting conflict files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_007";

    // Note: When the src path (testDir007) ends with "/", the contents (sub files) of the directory will be moved,
    // but not the directory itself.
    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_007/src/testDir007/";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_007/dest";
    auto srcSubFile = src + "/testFile.txt";
    auto destSubFile = dest + "/testFile.txt";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateFile(srcSubFile, "src content"));
    ASSERT_TRUE(FileUtils::CreateFile(destSubFile, "dest content"));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_FILE_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900015);
    EXPECT_EQ(err.GetErrMsg(), "File exists");
    EXPECT_TRUE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_007";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_008
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when src and dest have the same name but src
 * is a file while dest is a directory
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_008";

    // Note: When the src path (testDir008) ends with "/", the contents (sub files) of the directory will be moved,
    // but not the directory itself.
    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_008/src/testDir008/";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_008/dest";
    auto srcFile = src + "/theSameName";     // This is a file
    auto destSubDir = dest + "/theSameName"; // This is a directory
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateFile(srcFile, "content"));
    ASSERT_TRUE(FileUtils::CreateDirectories(destSubDir));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_008";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_009
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for SUCCESS when the same-named file exist in the target
 * directory will be overwritten.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_009";

    // Note: When the src path (testDir009) ends with "/", the contents (sub files) of the directory will be moved,
    // but not the directory itself.
    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_008/src/testDir009/";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_008/dest";
    auto srcSubFile = src + "/subFile.txt";
    auto destSubFile = dest + "/subFile.txt";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateFile(srcSubFile, "src content"));
    ASSERT_TRUE(FileUtils::CreateFile(destSubFile, "dest content"));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());
    EXPECT_EQ(FileUtils::ReadTextFileContent(destSubFile), std::make_tuple(true, "src content"));

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_009";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_010
 * @tc.desc: Test function of MoveDirCore::DoMoveDir interface for FAILURE when src and dest have the same name but src
 * is a directory while dest is a file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_010";

    // Note: When the src path (testDir010) ends with "/", the contents (sub files) of the directory will be moved,
    // but not the directory itself.
    auto src = testDir + "/MoveDirCoreTest_DoMoveDir_010/src/testDir010/";
    auto dest = testDir + "/MoveDirCoreTest_DoMoveDir_010/dest";
    auto srcSubDir = src + "/theSameName";    // This is a directory
    auto destSubFile = dest + "/theSameName"; // This is a file
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));
    ASSERT_TRUE(FileUtils::CreateDirectories(srcSubDir));
    ASSERT_TRUE(FileUtils::CreateFile(destSubFile, "content"));

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_010";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test