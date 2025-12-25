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

#include "rename_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RenameCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ReadTextCoreTest";
};

void RenameCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "RenameCoreTest");
}

void RenameCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void RenameCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void RenameCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RenameCoreTest_DoRename_001
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS when renaming a file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_001";

    string src = testDir + "/RenameCoreTest_DoRename_001_src.txt";
    string dest = testDir + "/RenameCoreTest_DoRename_001_dest.txt";
    ASSERT_TRUE(FileUtils::CreateFile(src, "content"));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(src));
    EXPECT_EQ(FileUtils::ReadTextFileContent(dest), std::make_tuple(true, "content"));

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_001";
}

/**
 * @tc.name: RenameCoreTest_DoRename_002
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS when the dest file is already exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_002";

    string src = testDir + "/RenameCoreTest_DoRename_002_src.txt";
    string dest = testDir + "/RenameCoreTest_DoRename_002_dest.txt";
    ASSERT_TRUE(FileUtils::CreateFile(src, "src content"));
    ASSERT_TRUE(FileUtils::CreateFile(dest, "dest content"));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(src));
    EXPECT_EQ(FileUtils::ReadTextFileContent(dest), std::make_tuple(true, "src content"));

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_002";
}

/**
 * @tc.name: RenameCoreTest_DoRename_003
 * @tc.desc: Test function of RenameCore::DoRename interface for FAILURE when dest path is already exist, but is not
 * file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_003";

    string src = testDir + "/RenameCoreTest_DoRename_003_src.txt";
    string dest = testDir + "/RenameCoreTest_DoRename_003_dest";
    ASSERT_TRUE(FileUtils::CreateFile(src, "content"));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900019);
    EXPECT_EQ(err.GetErrMsg(), "Is a directory");

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_003";
}

/**
 * @tc.name: RenameCoreTest_DoRename_004
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS when renaming a directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_004";

    string src = testDir + "/RenameCoreTest_DoRename_004_src";
    string dest = testDir + "/RenameCoreTest_DoRename_004_dest";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(src));
    EXPECT_TRUE(FileUtils::Exists(dest));

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_004";
}

/**
 * @tc.name: RenameCoreTest_DoRename_005
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS when the dest directory is already exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_005";

    string src = testDir + "/RenameCoreTest_DoRename_005_src";
    string dest = testDir + "/RenameCoreTest_DoRename_005_dest";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(src));
    EXPECT_TRUE(FileUtils::Exists(dest));

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_005";
}

/**
 * @tc.name: RenameCoreTest_DoRename_006
 * @tc.desc: Test function of RenameCore::DoRename interface for FAILURE when dest path is already exits, but is not a
 * directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_006";

    string src = testDir + "/RenameCoreTest_DoRename_006_src";
    string dest = testDir + "/RenameCoreTest_DoRename_006_dest.txt";
    ASSERT_TRUE(FileUtils::CreateDirectories(src));
    ASSERT_TRUE(FileUtils::CreateFile(dest, "content"));

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900018);
    EXPECT_EQ(err.GetErrMsg(), "Not a directory");

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_006";
}

/**
 * @tc.name: RenameCoreTest_DoRename_007
 * @tc.desc: Test function of RenameCore::DoRename interface for FAILURE when src path is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_007";

    string src = testDir + "/RenameCoreTest_DoRename_007_src_not_existent.txt";
    string dest = testDir + "/RenameCoreTest_DoRename_007_dest.txt";

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_007";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test