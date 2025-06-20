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
#include "uv_fs_mock.h"

#include <cstring>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class StatCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock>  uvfs = nullptr;
};

void StatCoreMockTest::SetUpTestCase(void)
{
    uvfs = std::make_shared<UvfsMock>();
    Uvfs::ins = uvfs;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void StatCoreMockTest::TearDownTestCase(void)
{
    Uvfs::ins = nullptr;
    uvfs = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void StatCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_001
 * @tc.desc: Test function of StatCore::DoStat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_001";

    FileInfo fileinfo;
    fileinfo.path = std::make_unique<char[]>(1);
    fileinfo.isPath = true;

    EXPECT_CALL(*uvfs, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_001";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_002
 * @tc.desc: Test function of StatCore::DoStat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_002";

    FileInfo fileinfo;
    fileinfo.path = std::make_unique<char[]>(1);
    fileinfo.isPath = true;

    EXPECT_CALL(*uvfs, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_002";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_003
 * @tc.desc: Test function of StatCore::DoStat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_003";

    FileInfo fileinfo;
    fileinfo.path = std::make_unique<char[]>(1);
    fileinfo.isPath = false;

    EXPECT_CALL(*uvfs, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_003";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_004
 * @tc.desc: Test function of StatCore::DoStat interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_004";

    FileInfo fileinfo;
    fileinfo.path = std::make_unique<char[]>(1);
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);
    fileinfo.isPath = false;

    EXPECT_CALL(*uvfs, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = StatCore::DoStat(fileinfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test