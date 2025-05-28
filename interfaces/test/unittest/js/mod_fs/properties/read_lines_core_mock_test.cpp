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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock/uv_fs_mock.h"
#include "read_lines_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadLinesCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

filesystem::path ReadLinesCoreMockTest::tempFilePath;

void ReadLinesCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = filesystem::temp_directory_path() / "read_lines_test_file.txt";
    ofstream(tempFilePath) << "Test content\n123\n456";
    ofstream(tempFilePath).close();
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void ReadLinesCoreMockTest::TearDownTestCase(void)
{
    filesystem::remove(tempFilePath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void ReadLinesCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ReadLinesCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_001
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_001";

    string path = tempFilePath.string();
    Options option;
    option.encoding = "utf-8";

    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    auto res = ReadLinesCore::DoReadLines(path, option);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_001";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_002
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_002";

    string path = tempFilePath.string();

    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    auto res = ReadLinesCore::DoReadLines(path);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_002";
}

/**
 * @tc.name: ReadLinesCoreMockTest_DoReadLines_003
 * @tc.desc: Test function of ReadLinesCore::DoReadLines interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesCoreMockTest, ReadLinesCoreMockTest_DoReadLines_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-begin ReadLinesCoreMockTest_DoReadLines_003";

    string path = tempFilePath.string();
    Options option;
    option.encoding = "utf-8";

    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));
    auto res = ReadLinesCore::DoReadLines(path, option);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "ReadLinesCoreMockTest-end ReadLinesCoreMockTest_DoReadLines_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test