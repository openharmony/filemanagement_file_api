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

#include "unlink_core.h"

#include <filesystem>
#include <fstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UnlinkCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path UnlinkCoreMockTest::tempFilePath;

void UnlinkCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "UnlinkCoreMockTest");
    tempFilePath = filesystem::temp_directory_path() / "unlink_test_file.txt";
    ofstream(tempFilePath) << "Test content\n123\n456";
    UvFsMock::EnableMock();
}

void UnlinkCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    filesystem::remove(tempFilePath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UnlinkCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UnlinkCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: UnlinkCoreMockTest_DoUnlink_001
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreMockTest, UnlinkCoreMockTest_DoUnlink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreMockTest-begin UnlinkCoreMockTest_DoUnlink_001";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(1));

    string path = tempFilePath.string();
    auto res = UnlinkCore::DoUnlink(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "UnlinkCoreMockTest-end UnlinkCoreMockTest_DoUnlink_001";
}

/**
 * @tc.name: UnlinkCoreMockTest_DoUnlink_002
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreMockTest, UnlinkCoreMockTest_DoUnlink_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreMockTest-begin UnlinkCoreMockTest_DoUnlink_002";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1));

    string path = tempFilePath.string();
    auto res = UnlinkCore::DoUnlink(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreMockTest-end UnlinkCoreMockTest_DoUnlink_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test