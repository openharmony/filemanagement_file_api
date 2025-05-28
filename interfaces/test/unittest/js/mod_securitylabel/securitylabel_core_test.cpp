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

#define FILE_PATH "/data/test/SecurityLabelCoreTest.txt"

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
class SecurityLabelCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(FILE_PATH, O_CREAT | O_RDWR, 0644);
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(FILE_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: DoSetSecurityLabel_0001
 * @tc.desc: Test function of DoSetSecurityLabel() interface for invalid level.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetSecurityLabel_0001";
    auto ret = DoSetSecurityLabel(FILE_PATH, "abc");
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
 * @tc.require: AR000IGDNF
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetDoSetSecurityLabel_0002SecurityLabel_0001";
    auto ret = DoSetSecurityLabel("FILE_PATH", "s1");
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
 * @tc.require: AR000IGDNF
 */
HWTEST_F(SecurityLabelCoreTest, DoSetSecurityLabel_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoSetSecurityLabel_0003";
    auto ret = DoSetSecurityLabel(FILE_PATH, "s2");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoSetSecurityLabel_0003";
}

/**
 * @tc.name: DoGetSecurityLabel_0001
 * @tc.desc: Test function of DoGetSecurityLabel() interface for invalid path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(SecurityLabelCoreTest, DoGetSecurityLabel_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoGetSecurityLabel_0001";
    auto ret = DoGetSecurityLabel("FILE_PATH");
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
 * @tc.require: AR000IGDNF
 */
HWTEST_F(SecurityLabelCoreTest, DoGetSecurityLabel_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-begin DoGetSecurityLabel_0002";
    auto ret = DoGetSecurityLabel(FILE_PATH);
    EXPECT_TRUE(ret.IsSuccess());

    const string level = ret.GetData().value();
    EXPECT_EQ(level, "s2");

    GTEST_LOG_(INFO) << "SecurityLabelCoreTest-end DoGetSecurityLabel_0002";
}

} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS