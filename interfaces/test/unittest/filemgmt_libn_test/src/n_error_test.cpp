/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <string>

#include <gtest/gtest.h>

#include "n_error.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;

class AnonymizePathTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void AnonymizePathTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
}

void AnonymizePathTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void AnonymizePathTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AnonymizePathTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AnonymizePathTest_EmptyPath_0000
 * @tc.desc: Test AnonymizePath() with empty path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_EmptyPath_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_EmptyPath_0000";

    string result = AnonymizePath("");
    EXPECT_TRUE(result == "");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_EmptyPath_0000";
}

/**
 * @tc.name: AnonymizePathTest_SimplePath_0001
 * @tc.desc: Test AnonymizePath() with a simple absolute path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_SimplePath_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_SimplePath_0001";

    string result = AnonymizePath("/data/test");
    EXPECT_TRUE(result == "/d**a/t**t");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_SimplePath_0001";
}

/**
 * @tc.name: AnonymizePathTest_WithExtension_0002
 * @tc.desc: Test AnonymizePath() with a file that has an extension.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_WithExtension_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_WithExtension_0002";

    string result = AnonymizePath("/data/test/readme.txt");
    EXPECT_TRUE(result == "/d**a/t**t/re***e.t**");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_WithExtension_0002";
}

/**
 * @tc.name: AnonymizePathTest_WithUriScheme_0003
 * @tc.desc: Test AnonymizePath() with a URI scheme.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_WithUriScheme_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_WithUriScheme_0003";

    string result = AnonymizePath("file:///data/secret.log");
    EXPECT_TRUE(result == "file:///d**a/se***t.l**");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_WithUriScheme_0003";
}

/**
 * @tc.name: AnonymizePathTest_ShortName_0004
 * @tc.desc: Test AnonymizePath() with short component names (2-3 chars).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_ShortName_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_ShortName_0004";

    string result = AnonymizePath("ab/cd/ef");
    EXPECT_TRUE(result == "a*/c*/e*");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_ShortName_0004";
}

/**
 * @tc.name: AnonymizePathTest_SingleCharName_0005
 * @tc.desc: Test AnonymizePath() with single character component names.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_SingleCharName_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_SingleCharName_0005";

    string result = AnonymizePath("/a/b/c");
    EXPECT_TRUE(result == "/*/*/*");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_SingleCharName_0005";
}

/**
 * @tc.name: AnonymizePathTest_LongName_0006
 * @tc.desc: Test AnonymizePath() with long component names (>= 6 chars).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_LongName_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_LongName_0006";

    string result = AnonymizePath("abcdefgh");
    EXPECT_TRUE(result == "ab*****h");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_LongName_0006";
}

/**
 * @tc.name: AnonymizePathTest_FileUri_0007
 * @tc.desc: Test AnonymizePath() with a file URI.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_FileUri_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_FileUri_0007";

    string result = AnonymizePath("file://media/com.example.provider/data/file.txt");
    EXPECT_TRUE(result == "file://m***a/co*****************r/d**a/f**e.t**");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_FileUri_0007";
}

/**
 * @tc.name: AnonymizePathTest_DeepPath_0008
 * @tc.desc: Test AnonymizePath() with a deep directory path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AnonymizePathTest, AnonymizePathTest_DeepPath_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AnonymizePathTest-begin AnonymizePathTest_DeepPath_0008";

    string result = AnonymizePath("/usr/local/share/doc/index.html");
    EXPECT_TRUE(result == "/u**/l***l/s***e/d**/i***x.h**l");

    GTEST_LOG_(INFO) << "AnonymizePathTest-end AnonymizePathTest_DeepPath_0008";
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
