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

#include "lseek_core.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class LseekCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LseekCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void LseekCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void LseekCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LseekCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LseekCoreTest_DoLseek_001
 * @tc.desc: Test function of LseekCore::DoLseek interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LseekCoreTest, LseekCoreTest_DoLseek_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekCoreTest-begin LseekCoreTest_DoLseek_001";

    int32_t fd = -1;
    int64_t offset = 0;

    auto res = LseekCore::DoLseek(fd, offset);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "LseekCoreTest-end LseekCoreTest_DoLseek_001";
}

/**
 * @tc.name: LseekCoreTest_DoLseek_002
 * @tc.desc: Test function of LseekCore::DoLseek interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LseekCoreTest, LseekCoreTest_DoLseek_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekCoreTest-begin LseekCoreTest_DoLseek_002";

    int32_t fd = 1;
    int64_t offset = 0;
    optional<SeekPos> pos = std::make_optional(static_cast<SeekPos>(-1));

    auto res = LseekCore::DoLseek(fd, offset, pos);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "LseekCoreTest-end LseekCoreTest_DoLseek_002";
}

/**
 * @tc.name: LseekCoreTest_DoLseek_003
 * @tc.desc: Test function of LseekCore::DoLseek interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LseekCoreTest, LseekCoreTest_DoLseek_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekCoreTest-begin LseekCoreTest_DoLseek_003";

    int32_t fd = 1;
    int64_t offset = 0;
    optional<SeekPos> pos = std::make_optional(SeekPos::CURRENT);

    auto res = LseekCore::DoLseek(fd, offset, pos);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "LseekCoreTest-end LseekCoreTest_DoLseek_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test