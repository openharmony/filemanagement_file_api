/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsUtilsTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsUtilsTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsUtilsTest");
}

void FsUtilsTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsUtilsTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsUtilsTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsUtils_ConvertFlags_Uncache_001
 * @tc.desc: Test function of FsUtils::ConvertFlags with UNCACHE flag only.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsUtilsTest, FsUtils_ConvertFlags_Uncache_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsUtilsTest-begin FsUtils_ConvertFlags_Uncache_001";

    uint32_t flags = USR_UNCACHE;
    uint32_t result = FsUtils::ConvertFlags(flags);

    EXPECT_EQ(result & static_cast<uint32_t>(UNCACHE), static_cast<uint32_t>(UNCACHE));

    GTEST_LOG_(INFO) << "FsUtilsTest-end FsUtils_ConvertFlags_Uncache_001";
}

/**
 * @tc.name: FsUtils_ConvertFlags_Uncache_002
 * @tc.desc: Test function of FsUtils::ConvertFlags with UNCACHE combined with other flags.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsUtilsTest, FsUtils_ConvertFlags_Uncache_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsUtilsTest-begin FsUtils_ConvertFlags_Uncache_002";

    uint32_t flags = USR_UNCACHE | USR_CREATE | USR_RDWR;
    uint32_t result = FsUtils::ConvertFlags(flags);

    EXPECT_EQ(result & static_cast<uint32_t>(UNCACHE), static_cast<uint32_t>(UNCACHE));
    EXPECT_EQ(result & static_cast<uint32_t>(CREATE), static_cast<uint32_t>(CREATE));

    GTEST_LOG_(INFO) << "FsUtilsTest-end FsUtils_ConvertFlags_Uncache_002";
}

/**
 * @tc.name: FsUtils_ConvertFlags_Uncache_003
 * @tc.desc: Test function of FsUtils::ConvertFlags without UNCACHE flag.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsUtilsTest, FsUtils_ConvertFlags_Uncache_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsUtilsTest-begin FsUtils_ConvertFlags_Uncache_003";

    uint32_t flags = USR_CREATE | USR_RDWR | USR_TRUNC;
    uint32_t result = FsUtils::ConvertFlags(flags);

    EXPECT_EQ(result & static_cast<uint32_t>(UNCACHE), 0U);

    GTEST_LOG_(INFO) << "FsUtilsTest-end FsUtils_ConvertFlags_Uncache_003";
}

/**
 * @tc.name: FsUtils_UncacheConstantValue_001
 * @tc.desc: Test UNCACHE constant value is correct and unique.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsUtilsTest, FsUtils_UncacheConstantValue_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsUtilsTest-begin FsUtils_UncacheConstantValue_001";

    EXPECT_EQ(UNCACHE, 010000000000);
    EXPECT_EQ(USR_UNCACHE, 010000000000U);

    EXPECT_NE(UNCACHE, 0);
    EXPECT_NE(USR_UNCACHE, 0U);

    EXPECT_NE(UNCACHE, CREATE);
    EXPECT_NE(UNCACHE, TRUNC);
    EXPECT_NE(UNCACHE, APPEND);
    EXPECT_NE(UNCACHE, SYNC);

    GTEST_LOG_(INFO) << "FsUtilsTest-end FsUtils_UncacheConstantValue_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
