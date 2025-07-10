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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/uv_fs_mock.h"
#include "unlink_core.h"

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
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

filesystem::path UnlinkCoreMockTest::tempFilePath;

void UnlinkCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = filesystem::temp_directory_path() / "unlink_test_file.txt";
    ofstream(tempFilePath) << "unlink_test_file";
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void UnlinkCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove(tempFilePath);
    Uvfs::ins = nullptr;
    uvMock = nullptr;
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

    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(1));

    string path = tempFilePath.string();
    auto res = UnlinkCore::DoUnlink(path);
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

    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1));

    string path = tempFilePath.string();
    auto res = UnlinkCore::DoUnlink(path);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreMockTest-end UnlinkCoreMockTest_DoUnlink_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test