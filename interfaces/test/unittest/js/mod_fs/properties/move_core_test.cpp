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

#include "move_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/MoveCoreTest";
};

void MoveCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MoveCoreTest");
}

void MoveCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MoveCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void MoveCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MoveCoreTest_DoMove_001
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when src path and dest path are all empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_001";

    std::string src = "";
    std::string dest = "";

    auto res = MoveCore::DoMove(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_001";
}

/**
 * @tc.name: MoveCoreTest_DoMove_002
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when dest path is empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_002";

    std::string src = testDir + "/MoveCoreTest_DoMove_002_src.txt";
    std::string dest = "";

    auto res = MoveCore::DoMove(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_002";
}

/**
 * @tc.name: MoveCoreTest_DoMove_003
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_003";

    std::string src = testDir + "/MoveCoreTest_DoMove_003_src.txt";
    std::string dest = testDir + "/MoveCoreTest_DoMove_003_dest.txt";
    int mode = 3; // invalid mode

    auto res = MoveCore::DoMove(src, dest, mode);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_003";
}

/**
 * @tc.name: MoveCoreTest_DoMove_004
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when src path is a directory (invalid src).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_004";

    std::string src = testDir + "/MoveCoreTest_DoMove_004_src";
    std::string dest = testDir + "/MoveCoreTest_DoMove_004_dest.txt";
    int mode = MODE_FORCE_MOVE;

    ASSERT_TRUE(FileUtils::CreateDirectories(src));

    auto res = MoveCore::DoMove(src, dest, mode);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_004";
}

/**
 * @tc.name: MoveCoreTest_DoMove_005
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when dest path is a directory (invalid dest).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_005";

    std::string src = testDir + "/MoveCoreTest_DoMove_005_src.txt";
    std::string dest = testDir + "/MoveCoreTest_DoMove_005_dest";
    int mode = MODE_FORCE_MOVE;

    ASSERT_TRUE(FileUtils::CreateDirectories(dest));

    auto res = MoveCore::DoMove(src, dest, mode);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test