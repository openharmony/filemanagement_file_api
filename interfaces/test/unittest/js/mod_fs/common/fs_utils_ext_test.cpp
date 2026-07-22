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
#include <optional>
#include <sys/prctl.h>

#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsUtilsExtTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsUtilsExtTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsUtilsExtTest");
}

void FsUtilsExtTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsUtilsExtTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsUtilsExtTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetActualLen_001
 * @tc.desc: Test FsUtils::GetActualLen with no optional length, normal case.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetActualLen_001 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 0);
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 100);

    GTEST_LOG_(INFO) << "GetActualLen_001 end";
}

/**
 * @tc.name: GetActualLen_002
 * @tc.desc: Test FsUtils::GetActualLen with offset, no optional length.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetActualLen_002 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 30);
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 70);

    GTEST_LOG_(INFO) << "GetActualLen_002 end";
}

/**
 * @tc.name: GetActualLen_003
 * @tc.desc: Test FsUtils::GetActualLen with bufLen less than bufOff, should fail.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_003, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_003 start";

    auto [succ, len] = FsUtils::GetActualLen(10, 50);
    EXPECT_FALSE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_003 end";
}

/**
 * @tc.name: GetActualLen_004
 * @tc.desc: Test FsUtils::GetActualLen with valid optional length.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetActualLen_004 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 0, optional<int64_t>(50));
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 50);

    GTEST_LOG_(INFO) << "GetActualLen_004 end";
}

/**
 * @tc.name: GetActualLen_005
 * @tc.desc: Test FsUtils::GetActualLen with optional length equal to remaining buffer size.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetActualLen_005 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 20, optional<int64_t>(80));
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 80);

    GTEST_LOG_(INFO) << "GetActualLen_005 end";
}

/**
 * @tc.name: GetActualLen_006
 * @tc.desc: Test FsUtils::GetActualLen with negative optional length, should fail.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_006, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_006 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 0, optional<int64_t>(-1));
    EXPECT_FALSE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_006 end";
}

/**
 * @tc.name: GetActualLen_007
 * @tc.desc: Test FsUtils::GetActualLen with optional length exceeding remaining buffer size, should fail.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_007, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_007 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 20, optional<int64_t>(200));
    EXPECT_FALSE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_007 end";
}

/**
 * @tc.name: GetActualLen_008
 * @tc.desc: Test FsUtils::GetActualLen with zero optional length.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_008, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_008 start";

    auto [succ, len] = FsUtils::GetActualLen(100, 0, optional<int64_t>(0));
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_008 end";
}

/**
 * @tc.name: GetActualLen_009
 * @tc.desc: Test FsUtils::GetActualLen with zero buffer length and zero offset, nullopt length.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_009, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_009 start";

    auto [succ, len] = FsUtils::GetActualLen(0, 0);
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_009 end";
}

/**
 * @tc.name: GetActualLen_010
 * @tc.desc: Test FsUtils::GetActualLen with equal bufLen and bufOff, nullopt length.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetActualLen_010, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetActualLen_010 start";

    auto [succ, len] = FsUtils::GetActualLen(50, 50);
    EXPECT_TRUE(succ);
    EXPECT_EQ(len, 0);

    GTEST_LOG_(INFO) << "GetActualLen_010 end";
}

/**
 * @tc.name: GetModeFromFlags_001
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDONLY flag.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_001 start";

    string mode = FsUtils::GetModeFromFlags(O_RDONLY);
    EXPECT_EQ(mode, "r");

    GTEST_LOG_(INFO) << "GetModeFromFlags_001 end";
}

/**
 * @tc.name: GetModeFromFlags_002
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_WRONLY flag.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_002 start";

    string mode = FsUtils::GetModeFromFlags(O_WRONLY);
    EXPECT_EQ(mode, "w");

    GTEST_LOG_(INFO) << "GetModeFromFlags_002 end";
}

/**
 * @tc.name: GetModeFromFlags_003
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDWR flag.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_003 start";

    string mode = FsUtils::GetModeFromFlags(O_RDWR);
    EXPECT_EQ(mode, "rw");

    GTEST_LOG_(INFO) << "GetModeFromFlags_003 end";
}

/**
 * @tc.name: GetModeFromFlags_004
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDWR | O_TRUNC flags.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_004 start";

    string mode = FsUtils::GetModeFromFlags(O_RDWR | O_TRUNC);
    EXPECT_EQ(mode, "rwt");

    GTEST_LOG_(INFO) << "GetModeFromFlags_004 end";
}

/**
 * @tc.name: GetModeFromFlags_005
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_WRONLY | O_APPEND flags.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_005 start";

    string mode = FsUtils::GetModeFromFlags(O_WRONLY | O_APPEND);
    EXPECT_EQ(mode, "wa");

    GTEST_LOG_(INFO) << "GetModeFromFlags_005 end";
}

/**
 * @tc.name: GetModeFromFlags_006
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDWR | O_APPEND flags.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_006 start";

    string mode = FsUtils::GetModeFromFlags(O_RDWR | O_APPEND);
    EXPECT_EQ(mode, "rwa");

    GTEST_LOG_(INFO) << "GetModeFromFlags_006 end";
}

/**
 * @tc.name: GetModeFromFlags_007
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_WRONLY | O_TRUNC flags.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_007 start";

    string mode = FsUtils::GetModeFromFlags(O_WRONLY | O_TRUNC);
    EXPECT_EQ(mode, "wt");

    GTEST_LOG_(INFO) << "GetModeFromFlags_007 end";
}

/**
 * @tc.name: GetModeFromFlags_008
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDWR | O_TRUNC | O_APPEND flags.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_008, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_008 start";

    string mode = FsUtils::GetModeFromFlags(O_RDWR | O_TRUNC | O_APPEND);
    EXPECT_EQ(mode, "rwta");

    GTEST_LOG_(INFO) << "GetModeFromFlags_008 end";
}

/**
 * @tc.name: GetModeFromFlags_009
 * @tc.desc: Test FsUtils::GetModeFromFlags with zero flags, defaults to read mode.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_009, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_009 start";

    string mode = FsUtils::GetModeFromFlags(0);
    EXPECT_EQ(mode, "r");

    GTEST_LOG_(INFO) << "GetModeFromFlags_009 end";
}

/**
 * @tc.name: GetModeFromFlags_010
 * @tc.desc: Test FsUtils::GetModeFromFlags with O_RDONLY | O_APPEND, read mode ignores append.
 * @tc.type: FUNC
 * @tc.require: issueI56WJ7
 */
HWTEST_F(FsUtilsExtTest, GetModeFromFlags_010, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "GetModeFromFlags_010 start";

    string mode = FsUtils::GetModeFromFlags(O_RDONLY | O_APPEND);
    EXPECT_EQ(mode, "r");

    GTEST_LOG_(INFO) << "GetModeFromFlags_010 end";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
