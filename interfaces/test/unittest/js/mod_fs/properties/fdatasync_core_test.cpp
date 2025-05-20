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

#include "fdatasync_core.h"
#include "mock/uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FDataSyncCoreTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path FDataSyncCoreTest::tempFilePath;

void FDataSyncCoreTest::SetUpTestCase(void)
{
    tempFilePath = filesystem::temp_directory_path() / "fdatasync_test_file.txt";
    ofstream(tempFilePath) << "Test content\n123\n456";
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FDataSyncCoreTest::TearDownTestCase(void)
{
    filesystem::remove(tempFilePath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FDataSyncCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FDataSyncCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FDataSyncCoreTest_DoFDataSync_001
 * @tc.desc: Test function of FDataSyncCore::DoFDataSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FDataSyncCoreTest, FDataSyncCoreTest_DoFDataSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FDataSyncCoreTest_DoFDataSync_001";

    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;
    EXPECT_CALL(*uv, uv_fs_fdatasync(_, _, _, _)).WillOnce(Return(1));

    string path = tempFilePath.string();
    auto fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
    auto res = FDataSyncCore::DoFDataSync(fd);
    EXPECT_EQ(res.IsSuccess(), true);
    close(fd);

    GTEST_LOG_(INFO) << "NClassTest-end FDataSyncCoreTest_DoFDataSync_001";
}

/**
 * @tc.name: FDataSyncCoreTest_DoFDataSync_002
 * @tc.desc: Test function of FDataSyncCore::DoFDataSync interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FDataSyncCoreTest, FDataSyncCoreTest_DoFDataSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FDataSyncCoreTest_DoFDataSync_002";

    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;
    EXPECT_CALL(*uv, uv_fs_fdatasync(_, _, _, _)).WillOnce(Return(-1));

    string path = tempFilePath.string();
    auto fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
    auto res = FDataSyncCore::DoFDataSync(fd);
    EXPECT_EQ(res.IsSuccess(), false);
    close(fd);

    GTEST_LOG_(INFO) << "NClassTest-end FDataSyncCoreTest_DoFDataSync_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test