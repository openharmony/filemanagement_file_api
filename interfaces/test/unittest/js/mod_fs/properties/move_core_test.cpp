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

#include "move_core.h"

#include <filesystem>
#include <gtest/gtest.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MoveCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void MoveCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void MoveCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MoveCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MoveCoreTest_DoMove_001
 * @tc.desc: Test function of MoveCore::DoMove interface for ERROR.
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
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_001";
}

/**
 * @tc.name: MoveCoreTest_DoMove_002
 * @tc.desc: Test function of MoveCore::DoMove interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_002";

    std::string src = "/src.txt";
    std::string dest = "";

    auto res = MoveCore::DoMove(src, dest);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_002";
}

/**
 * @tc.name: MoveCoreTest_DoMove_003
 * @tc.desc: Test function of MoveCore::DoMove interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_003";

    std::string src = "/src.txt";
    std::string dest = "/dest.txt";
    int mode = 3;

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_003";
}

/**
 * @tc.name: MoveCoreTest_DoMove_004
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_004";

    std::string src = "dir";
    std::string dest = "/dest.txt";
    int mode = 3;

    std::filesystem::path dirpath = "dir";
    ASSERT_TRUE(std::filesystem::create_directories(dirpath));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    std::filesystem::remove(dirpath);
    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_004";
}

/**
 * @tc.name: MoveCoreTest_DoMove_005
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreTest, MoveCoreTest_DoMove_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreTest-begin MoveCoreTest_DoMove_005";

    std::string src = "/src.txt";
    std::string dest = "dir";
    int mode = 3;

    std::filesystem::path dirpath = "dir";
    ASSERT_TRUE(std::filesystem::create_directories(dirpath));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    std::filesystem::remove(dirpath);
    GTEST_LOG_(INFO) << "MoveCoreTest-end MoveCoreTest_DoMove_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test