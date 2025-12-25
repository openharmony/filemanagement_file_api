/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common_func.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#define FD_SAN_OVERFLOW_MAX 2047

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class CommonFuncTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void CommonFuncTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CommonFuncTest");
}

void CommonFuncTest::TearDownTestSuite(void)
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CommonFuncTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CommonFuncTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetFdTag_001
 * @tc.desc: Test function of CommonFunc::GetFdTag interfac fd<0 for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, GetFdTag_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin GetFdTag_001";

    uint64_t res = CommonFunc::GetFdTag(-1);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end GetFdTag_001";
}

/**
 * @tc.name: GetFdTag_002
 * @tc.desc: Test function of CommonFunc::GetFdTag interface if is fd for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, GetFdTag_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin GetFdTag_002";

    uint64_t expectTag = 1;
    CommonFunc::SetFdTag(FD_SAN_TABLE_SIZE, expectTag);
    uint64_t res = CommonFunc::GetFdTag(FD_SAN_TABLE_SIZE);

    EXPECT_EQ(res, 1);

    GTEST_LOG_(INFO) << "CommonFuncTest-end GetFdTag_002";
}

/**
 * @tc.name: GetFdTag_003
 * @tc.desc: Test function of CommonFunc::GetFdTag interface fd>2047 for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, GetFdTag_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin GetFdTag_003";

    uint64_t res = CommonFunc::GetFdTag(FD_SAN_OVERFLOW_MAX + 1);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end GetFdTag_003";
}

/**
 * @tc.name: SetFdTag_001
 * @tc.desc: Test function of CommonFunc::SetFdTag interface fd<0 for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, SetFdTag_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin SetFdTag_001";

    CommonFunc::SetFdTag(-1, 0);
    uint64_t res = CommonFunc::GetFdTag(-1);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end SetFdTag_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS