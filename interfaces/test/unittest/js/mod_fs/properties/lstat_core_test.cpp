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

#include <gtest/gtest.h>

#include "lstat_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class LstatCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LstatCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t fd = open("/data/test/lstat.txt", CREATE | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        ASSERT_TRUE(false);
    }
    close(fd);
}

void LstatCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    rmdir("/data/test/lstat.txt");
}

void LstatCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LstatCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatCoreTest_DoLstat_001
 * @tc.desc: Test function of LstatCore::DoLstat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreTest, LstatCoreTest_DoLstat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreTest-begin LstatCoreTest_DoLstat_001";

    auto res = LstatCore::DoLstat("/invalid/test/lstat.txt");
    EXPECT_EQ(res.IsSuccess(), false);
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "LstatCoreTest-end LstatCoreTest_DoLstat_001";
}

/**
 * @tc.name: LstatCoreTest_DoLstat_002
 * @tc.desc: Test function of LstatCore::DoLstat interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreTest, LstatCoreTest_DoLstat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreTest-begin LstatCoreTest_DoLstat_002";

    auto res = LstatCore::DoLstat("/data/test/lstat.txt");
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "LstatCoreTest-end LstatCoreTest_DoLstat_002";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
