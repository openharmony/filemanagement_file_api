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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include "securitylabel_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

static const string g_filePath = "/data/test/SecurityLabelCoreTest.txt";
static const string g_validFilePath = "/data/test/validFilePath";

class SecurityLabelCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void SecurityLabelCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    int32_t fd = open(g_filePath.c_str(), O_CREAT | O_RDWR, 0644);
    close(fd);
}

void SecurityLabelCoreTest::TearDownTestSuite()
{
    rmdir(g_filePath.c_str());
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void SecurityLabelCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SecurityLabelCoreTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DoSetSecurityLabel_0001
 * @tc.desc: Test function of DoSetSecurityLabel() interface for invalid level.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetSecurityLabel_0001";
    auto ret = DoSetSecurityLabel(g_filePath, "abc");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoSetSecurityLabel_0001";
}

/**
 * @tc.name: DoSetSecurityLabel_0002
 * @tc.desc: Test function of DoSetSecurityLabel() interface for invalid path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetSecurityLabel_0002";
    auto ret = DoSetSecurityLabel(g_validFilePath, "s1");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoSetSecurityLabel_0002";
}

/**
 * @tc.name: DoSetSecurityLabel_0003
 * @tc.desc: Test function of DoSetSecurityLabel() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetSecurityLabel_0003";
    auto ret = DoSetSecurityLabel(g_filePath, "s2");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoSetSecurityLabel_0003";
}

/**
 * @tc.name: DoGetSecurityLabel_0001
 * @tc.desc: Test function of DoGetSecurityLabel() interface for invalid path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, DoGetSecurityLabel_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoGetSecurityLabel_0001";
    auto ret = DoGetSecurityLabel(g_validFilePath);
    EXPECT_TRUE(ret.IsSuccess());

    const string level = ret.GetData().value();
    EXPECT_EQ(level, "s3");

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoGetSecurityLabel_0001";
}

/**
 * @tc.name: DoGetSecurityLabel_0002
 * @tc.desc: Test function of DoGetSecurityLabel() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SecurityLabelCoreTest, DoGetSecurityLabel_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoGetSecurityLabel_0002";
    auto ret = DoGetSecurityLabel(g_filePath);
    EXPECT_TRUE(ret.IsSuccess());

    const string level = ret.GetData().value();
    EXPECT_EQ(level, "s2");

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoGetSecurityLabel_0002";
}

} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS