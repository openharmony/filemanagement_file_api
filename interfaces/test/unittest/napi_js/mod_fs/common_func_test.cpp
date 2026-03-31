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

/**
 * @tc.name: ConvertJsFlags_UncacheFlag_001
 * @tc.desc: Test function of CommonFunc::ConvertJsFlags with UNCACHE flag only.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, ConvertJsFlags_UncacheFlag_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin ConvertJsFlags_UncacheFlag_001";

    unsigned int flags = USR_UNCACHE;
    unsigned int result = CommonFunc::ConvertJsFlags(flags);

    EXPECT_EQ(result & UNCACHE, static_cast<unsigned int>(UNCACHE));

    GTEST_LOG_(INFO) << "CommonFuncTest-end ConvertJsFlags_UncacheFlag_001";
}

/**
 * @tc.name: ConvertJsFlags_UncacheFlag_002
 * @tc.desc: Test function of CommonFunc::ConvertJsFlags with UNCACHE combined with CREATE flag.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, ConvertJsFlags_UncacheFlag_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin ConvertJsFlags_UncacheFlag_002";

    unsigned int flags = USR_UNCACHE | USR_CREATE;
    unsigned int result = CommonFunc::ConvertJsFlags(flags);

    EXPECT_EQ(result & UNCACHE, static_cast<unsigned int>(UNCACHE));
    EXPECT_EQ(result & CREATE, static_cast<unsigned int>(CREATE));

    GTEST_LOG_(INFO) << "CommonFuncTest-end ConvertJsFlags_UncacheFlag_002";
}

/**
 * @tc.name: ConvertJsFlags_UncacheFlag_003
 * @tc.desc: Test function of CommonFunc::ConvertJsFlags without UNCACHE flag.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, ConvertJsFlags_UncacheFlag_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin ConvertJsFlags_UncacheFlag_003";

    unsigned int flags = USR_CREATE | USR_RDWR;
    unsigned int result = CommonFunc::ConvertJsFlags(flags);

    EXPECT_EQ(result & UNCACHE, 0U);

    GTEST_LOG_(INFO) << "CommonFuncTest-end ConvertJsFlags_UncacheFlag_003";
}

/**
 * @tc.name: ConvertJsFlags_FlagsCombined_001
 * @tc.desc: Test UNCACHE flag combined with all other flags.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CommonFuncTest, ConvertJsFlags_FlagsCombined_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin ConvertJsFlags_FlagsCombined_001";

    unsigned int flags = USR_UNCACHE | USR_CREATE | USR_RDWR | USR_TRUNC | USR_APPEND | USR_SYNC;
    unsigned int result = CommonFunc::ConvertJsFlags(flags);

    EXPECT_EQ(result & UNCACHE, static_cast<unsigned int>(UNCACHE));
    EXPECT_EQ(result & CREATE, static_cast<unsigned int>(CREATE));
    EXPECT_EQ(result & RDWR, static_cast<unsigned int>(RDWR));
    EXPECT_EQ(result & TRUNC, static_cast<unsigned int>(TRUNC));
    EXPECT_EQ(result & APPEND, static_cast<unsigned int>(APPEND));
    EXPECT_EQ(result & SYNC, static_cast<unsigned int>(SYNC));

    GTEST_LOG_(INFO) << "CommonFuncTest-end ConvertJsFlags_FlagsCombined_001";
}

/**
 * @tc.name: UncacheConstantValue_001
 * @tc.desc: Test UNCACHE constant value is correct and unique.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, UncacheConstantValue_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin UncacheConstantValue_001";

    EXPECT_EQ(UNCACHE, 010000000000);
    EXPECT_EQ(USR_UNCACHE, 010000000000U);

    EXPECT_NE(UNCACHE, 0);
    EXPECT_NE(USR_UNCACHE, 0U);

    EXPECT_NE(UNCACHE, CREATE);
    EXPECT_NE(UNCACHE, TRUNC);
    EXPECT_NE(UNCACHE, APPEND);
    EXPECT_NE(UNCACHE, SYNC);

    GTEST_LOG_(INFO) << "CommonFuncTest-end UncacheConstantValue_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS