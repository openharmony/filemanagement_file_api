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

#include <cstring>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "securec.h"
#include "uv_fs_mock.h"

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
};

void StatCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "StatCoreMockTest");
    UvFsMock::EnableMock();
}

void StatCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
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

static bool SetPathForFileInfo(FileInfo &fileInfo, const string_view &path)
{
    auto len = path.length() + 1;
    auto pathPtr = std::make_unique<char[]>(len);
    int ret = strcpy_s(pathPtr.get(), len, path.data());
    if (ret != 0) {
        GTEST_LOG_(ERROR) << "SetPathForFileInfo failed! ret: " << ret;
        return false;
    }
    fileInfo.path = std::move(pathPtr);
    fileInfo.isPath = true;
    return true;
}

/**
 * @tc.name: StatCoreMockTest_DoStat_001
 * @tc.desc: Test function of StatCore::DoStat interface for FALSE when is path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_001";

    FileInfo fileinfo;
    fileinfo.fdg = nullptr;
    auto succ = SetPathForFileInfo(fileinfo, "StatCoreMockTest_DoStat_001");
    ASSERT_TRUE(succ);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_001";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_002
 * @tc.desc: Test function of StatCore::DoStat interface for SUCCESS when is path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_002";

    FileInfo fileinfo;
    fileinfo.fdg = nullptr;
    auto succ = SetPathForFileInfo(fileinfo, "StatCoreMockTest_DoStat_002");
    ASSERT_TRUE(succ);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_002";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_003
 * @tc.desc: Test function of StatCore::DoStat interface for FALSE when is FD.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_003";

    FileInfo fileinfo;
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileinfo.path = nullptr;
    fileinfo.isPath = false;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_003";
}

/**
 * @tc.name: StatCoreMockTest_DoStat_004
 * @tc.desc: Test function of StatCore::DoStat interface for SUCCESS when is FD.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatCoreMockTest, StatCoreMockTest_DoStat_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatCoreMockTest-begin StatCoreMockTest_DoStat_004";

    FileInfo fileinfo;
    fileinfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileinfo.path = nullptr;
    fileinfo.isPath = false;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_fstat(_, _, _, _)).WillOnce(Return(1));

    auto res = StatCore::DoStat(fileinfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "StatCoreMockTest-end StatCoreMockTest_DoStat_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test