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

#include "dup_core.h"

#include <fcntl.h>
#include <unistd.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DupCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DupCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DupCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DupCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DupCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DupCoreTest_DoDup_001
 * @tc.desc: Test function of DupCore::DoDup interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupCoreTest, DupCoreTest_DoDup_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin DupCoreTest_DoDup_001";
    int32_t fd = -1;
    auto res = DupCore::DoDup(fd);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "NClassTest-end DupCoreTest_DoDup_001";
}

/**
 * @tc.name: DupCoreTest_DoDup_002
 * @tc.desc: Test function of DupCore::DoDup interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupCoreTest, DupCoreTest_DoDup_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin DupCoreTest_DoDup_002";
    int32_t fd = open("temp_file.txt", O_CREAT | O_RDWR, 0666);
    ASSERT_NE(fd, -1);
    close(fd);

    auto res = DupCore::DoDup(fd);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "NClassTest-end DupCoreTest_DoDup_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test