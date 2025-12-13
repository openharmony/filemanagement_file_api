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

#include "rmdir_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RmdirCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ReadTextCoreTest";
};

void RmdirCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "RmdirCoreTest");
}

void RmdirCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RmdirCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void RmdirCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_001
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for FAILURE when path is empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_001";

    auto res = RmdirentCore::DoRmdirent("");

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_001";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_002
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for FAILURE when path is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_002";

    auto path = "\\invalid::path?*<>.txt";

    auto res = RmdirentCore::DoRmdirent(path);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_002";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_003
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for FAILURE when the directory is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_003";

    auto path = testDir + "/RmdirCoreTest_DoRmdirent_003_not_existent";

    auto res = RmdirentCore::DoRmdirent(path);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_003";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_004
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for SUCCESS when deleting an empty directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_004";

    auto path = testDir + "/RmdirCoreTest_DoRmdirent_004";
    ASSERT_TRUE(FileUtils::CreateDirectories(path));

    auto res = RmdirentCore::DoRmdirent(path);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(path));

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_004";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_005
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for SUCCESS when recursively deleting a non-empty
 * directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_005";

    auto path = testDir + "/RmdirCoreTest_DoRmdirent_005";
    auto subDir01 = path + "/subDir01";
    auto subDir02 = path + "/subDir02";
    auto subDir03 = subDir01 + "/subDir03";
    auto subFile01 = subDir02 + "/subFile01.txt";
    auto subFile02 = subDir03 + "/subFile02.txt";

    ASSERT_TRUE(FileUtils::CreateDirectories(path));
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir01));
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir02));
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir03));
    ASSERT_TRUE(FileUtils::CreateFile(subFile01, "subFile01 content"));
    ASSERT_TRUE(FileUtils::CreateFile(subFile02, "subFile02 content"));

    auto res = RmdirentCore::DoRmdirent(path);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(path));

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test