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

#include "mkdir_core.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MkdirCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

filesystem::path MkdirCoreMockTest::tempFilePath;

void MkdirCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = filesystem::temp_directory_path() / "mkdir_core_mock_test";
    std::filesystem::create_directory(tempFilePath);
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void MkdirCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove_all(tempFilePath);
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void MkdirCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MkdirCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_0001
 * @tc.desc: Test function of DoMkdir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_0001";

    EXPECT_CALL(*uvMock, uv_fs_mkdir(_, _, _, _, _)).WillOnce(Return(0));

    string path = tempFilePath.string() + "/test01";
    auto ret = MkdirCore::DoMkdir(path);
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_0001";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_0002
 * @tc.desc: Test function of DoMkdir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_0002";

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(-2)).WillOnce(Return(0));

    string path = tempFilePath.string() + "/test02/testDir";
    auto ret = MkdirCore::DoMkdir(path, true);
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_0002";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_0003
 * @tc.desc: Test function of DoMkdir() interface is FAILED for uv_fs_mkdir return 1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_0003";

    EXPECT_CALL(*uvMock, uv_fs_mkdir(_, _, _, _, _)).WillOnce(Return(1));

    string path = tempFilePath.string() + "/test03";
    auto ret = MkdirCore::DoMkdir(path);
    EXPECT_EQ(ret.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_0003";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_0004
 * @tc.desc: Test function of DoMkdir() interface is FAILED for file exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_0004";

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));

    string path = "/";
    auto ret = MkdirCore::DoMkdir(path, true);
    EXPECT_EQ(ret.IsSuccess(), false);
    auto err = ret.GetError();
    int errCode = err.GetErrNo();
    EXPECT_EQ(errCode, 13900015);
    auto msg = err.GetErrMsg();
    EXPECT_EQ(msg, "File exists");

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_0004";
}

/**
 * @tc.name: MkdirCoreMockTest_DoMkdir_0005
 * @tc.desc: Test function of DoMkdir() interface is FAILED for no such file or directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdirCoreMockTest, MkdirCoreMockTest_DoMkdir_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdirCoreMockTest-begin MkdirCoreMockTest_DoMkdir_0005";

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(2));

    string path = "";
    auto ret = MkdirCore::DoMkdir(path, true);
    EXPECT_EQ(ret.IsSuccess(), false);
    auto err = ret.GetError();
    int errCode = err.GetErrNo();
    EXPECT_EQ(errCode, 13900002);
    auto msg = err.GetErrMsg();
    EXPECT_EQ(msg, "No such file or directory");

    GTEST_LOG_(INFO) << "MkdirCoreMockTest-end MkdirCoreMockTest_DoMkdir_0005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test