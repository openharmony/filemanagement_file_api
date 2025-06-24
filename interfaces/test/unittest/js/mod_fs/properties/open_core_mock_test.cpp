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

#include "mock/uv_fs_mock.h"
#include "open_core.h"

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class OpenCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

void OpenCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void OpenCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void OpenCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void OpenCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_001
 * @tc.desc: Test function of OpenCore::DoOpen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_001";

    string path = "/test/open_test.txt";
    int32_t mode = 0;
    
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_001";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_002
 * @tc.desc: Test function of OpenCore::DoOpen interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_002";

    string path = "file://test/open_test.txt";
    int32_t mode = 0;

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_002";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_003
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_003";

    string path = "file://test/open_test.txt";
    int32_t mode = 0;

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-1));
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_003";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_004
 * @tc.desc: Test function of OpenCore::DoOpen interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_004";

    string path = "/test/open_test.txt";
    int32_t mode = 0;

    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-1));
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test