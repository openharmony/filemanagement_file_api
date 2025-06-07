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

#include "truncate_core.h"
#include "uv_fs_mock.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TruncateCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

void TruncateCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void TruncateCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void TruncateCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TruncateCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_001
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_001";

    FileInfo fileInfo;
    fileInfo.isPath = true;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-1));
    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_001";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_002
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_002";

    FileInfo fileInfo;
    fileInfo.isPath = true;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(-1));
    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_002";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_003
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_003";

    FileInfo fileInfo;
    fileInfo.isPath = true;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(1));
    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_003";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_004
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_004";

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(-1));
    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_004";
}

/**
 * @tc.name: TruncateCoreMockTest_DoTruncate_005
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreMockTest, TruncateCoreMockTest_DoTruncate_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreMockTest-begin TruncateCoreMockTest_DoTruncate_005";

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).WillOnce(Return(1));
    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "TruncateCoreMockTest-end TruncateCoreMockTest_DoTruncate_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test