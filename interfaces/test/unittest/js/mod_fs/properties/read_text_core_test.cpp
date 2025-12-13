/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "read_text_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

using namespace std;

class ReadTextCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ReadTextCoreTest";
};

void ReadTextCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "ReadTextCoreTest");
}

void ReadTextCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ReadTextCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void ReadTextCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_001
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_001";

    auto path = testDir + "/DoReadTextTest_DoReadText_001.txt";
    string content = "hello world";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    auto ret = ReadTextCore::DoReadText(path);

    ASSERT_TRUE(ret.IsSuccess());
    auto &[text, len] = ret.GetData().value();
    EXPECT_EQ(text, content);
    EXPECT_EQ(len, content.length());

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_001";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_002
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_002";

    auto path = testDir + "/DoReadTextTest_DoReadText_002.txt";
    ReadTextOptions options;
    options.offset = -1;

    auto ret = ReadTextCore::DoReadText(path, options);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_002";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_003
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_003";

    auto path = testDir + "/DoReadTextTest_DoReadText_003.txt";
    ReadTextOptions options;
    options.length = -1;

    auto ret = ReadTextCore::DoReadText(path, options);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_003";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_004
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when length > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_004";

    auto path = testDir + "/DoReadTextTest_DoReadText_004.txt";
    ReadTextOptions options;
    options.length = static_cast<int64_t>(UINT_MAX) + 1;

    auto ret = ReadTextCore::DoReadText(path, options);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_004";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_005
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when encoding is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_005";

    auto path = testDir + "/DoReadTextTest_DoReadText_005.txt";
    ReadTextOptions options;
    options.encoding = "gbk";

    auto ret = ReadTextCore::DoReadText(path, options);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_005";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_006
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when no such file or directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_006";

    auto path = testDir + "/DoReadTextTest_DoReadText_006_non_existent.txt";

    auto ret = ReadTextCore::DoReadText(path);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_006";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_007
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for FAILURE when offset is out of range.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_007";

    auto path = testDir + "/DoReadTextTest_DoReadText_007.txt";
    string content = "hello world";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    ReadTextOptions options;
    options.offset = content.length() + 1; // out of range

    auto ret = ReadTextCore::DoReadText(path, options);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_007";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_008
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for SUCCESS when then value of options.length is larger
 * than the actual size.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_008";

    auto path = testDir + "/DoReadTextTest_DoReadText_008.txt";
    string content = "hello world";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    ReadTextOptions options;
    options.length = content.length() * 2; // larger than actual size

    auto ret = ReadTextCore::DoReadText(path, options);

    ASSERT_TRUE(ret.IsSuccess());
    auto &[text, len] = ret.GetData().value();
    EXPECT_EQ(text, content);
    EXPECT_EQ(len, content.length());

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_008";
}

/**
 * @tc.name: DoReadTextTest_DoReadText_009
 * @tc.desc: Test function of ReadTextCore::DoReadText interface for SUCCESS with options offset and length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextCoreTest, DoReadTextTest_DoReadText_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_DoReadText_009";

    auto path = testDir + "/DoReadTextTest_DoReadText_009.txt";
    string content = "hello world DoReadTextTest_DoReadText_009";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    auto offset = 1;
    auto length = 5;
    ASSERT_LT(offset + length, content.length());
    ReadTextOptions options;
    options.offset = offset;
    options.length = length;

    auto ret = ReadTextCore::DoReadText(path, options);

    ASSERT_TRUE(ret.IsSuccess());
    auto &[text, len] = ret.GetData().value();
    EXPECT_EQ(text, content.substr(offset, length));
    EXPECT_EQ(len, length);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_DoReadText_009";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS