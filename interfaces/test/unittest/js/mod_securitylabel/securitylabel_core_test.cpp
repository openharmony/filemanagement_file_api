/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "securitylabel_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "security_label.h"
#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
namespace Test {
using namespace std;
using FileUtils = OHOS::FileManagement::ModuleFileIO::Test::FileUtils;

class SecurityLabelCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/SecurityLabelCoreTest";
};

void SecurityLabelCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "SecurityLabelCoreTest");
}

void SecurityLabelCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void SecurityLabelCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void SecurityLabelCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SecurityLabelCoreTest_DoSetSecurityLabel_001
 * @tc.desc: Test function of DoSetSecurityLabel interface for FAILURE when level is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, SecurityLabelCoreTest_DoSetSecurityLabel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin SecurityLabelCoreTest_DoSetSecurityLabel_001";

    auto path = testDir + "/SecurityLabelCoreTest_DoSetSecurityLabel_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto ret = DoSetSecurityLabel(path, "abc");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end SecurityLabelCoreTest_DoSetSecurityLabel_001";
}

/**
 * @tc.name: SecurityLabelCoreTest_DoSetSecurityLabel_002
 * @tc.desc: Test function of DoSetSecurityLabel interface for FAILURE when path not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, SecurityLabelCoreTest_DoSetSecurityLabel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin SecurityLabelCoreTest_DoSetSecurityLabel_002";

    auto path = testDir + "/SecurityLabelCoreTest_DoSetSecurityLabel_002_non_existent.txt";

    auto ret = DoSetSecurityLabel(path, "s1");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end SecurityLabelCoreTest_DoSetSecurityLabel_002";
}

/**
 * @tc.name: SecurityLabelCoreTest_DoSetSecurityLabel_003
 * @tc.desc: Test function of DoSetSecurityLabel interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, SecurityLabelCoreTest_DoSetSecurityLabel_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin SecurityLabelCoreTest_DoSetSecurityLabel_003";

    auto path = testDir + "/SecurityLabelCoreTest_DoSetSecurityLabel_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto expectedLevel = "s2";
    auto ret = DoSetSecurityLabel(path, expectedLevel);

    ASSERT_TRUE(ret.IsSuccess());
    string level = SecurityLabel::GetSecurityLabel(path);
    EXPECT_EQ(level, expectedLevel);

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end SecurityLabelCoreTest_DoSetSecurityLabel_003";
}

/**
 * @tc.name: SecurityLabelCoreTest_DoGetSecurityLabel_001
 * @tc.desc: Test function of DoGetSecurityLabel interface for SUCCESS when path not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, SecurityLabelCoreTest_DoGetSecurityLabel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin SecurityLabelCoreTest_DoGetSecurityLabel_001";

    auto path = testDir + "/SecurityLabelCoreTest_DoGetSecurityLabel_001_non_existent.txt";

    auto ret = DoGetSecurityLabel(path);

    ASSERT_TRUE(ret.IsSuccess());
    string level = ret.GetData().value();
    EXPECT_EQ(level, "s3"); // Default level: s3

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end SecurityLabelCoreTest_DoGetSecurityLabel_001";
}

/**
 * @tc.name: SecurityLabelCoreTest_DoGetSecurityLabel_002
 * @tc.desc: Test function of DoGetSecurityLabel interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, SecurityLabelCoreTest_DoGetSecurityLabel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin SecurityLabelCoreTest_DoGetSecurityLabel_002";

    auto path = testDir + "/SecurityLabelCoreTest_DoGetSecurityLabel_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    string expectedLevel = "s2";
    ASSERT_TRUE(SecurityLabel::SetSecurityLabel(path, expectedLevel));

    auto ret = DoGetSecurityLabel(path);

    ASSERT_TRUE(ret.IsSuccess());
    string level = ret.GetData().value();
    EXPECT_EQ(level, expectedLevel);

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end SecurityLabelCoreTest_DoGetSecurityLabel_002";
}

} // namespace Test
} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS