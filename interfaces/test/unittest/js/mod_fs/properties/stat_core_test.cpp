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

#include "stat_core.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class StatCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StatCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void StatCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void StatCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatCoreTest_DoStat_001
 * @tc.desc: Test function of FsyncCore::DoStat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreTest, StatCoreTest_DoStat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreTest-begin StatCoreTest_DoStat_001";

    FileInfo fileinfo;

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreTest-end StatCoreTest_DoStat_001";
}

/**
 * @tc.name: StatCoreTest_DoStat_002
 * @tc.desc: Test function of FsyncCore::DoStat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreTest, StatCoreTest_DoStat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreTest-begin StatCoreTest_DoStat_002";

    FileInfo fileinfo;
    fileinfo.path = std::make_unique<char[]>(1);
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(-1);
    fileinfo.isPath = false;

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreTest-end StatCoreTest_DoStat_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test