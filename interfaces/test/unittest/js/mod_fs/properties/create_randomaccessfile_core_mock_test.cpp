/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "create_randomaccessfile_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CreateRandomAccessFileCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void CreateRandomAccessFileCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CreateRandomAccessFileCoreMockTest");
    UvFsMock::EnableMock();
}

void CreateRandomAccessFileCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CreateRandomAccessFileCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CreateRandomAccessFileCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_001
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreMockTest, CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_001";

    string path = "fakePath/CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_001.txt";
    int32_t mode = 0;
    optional<RandomAccessFileOptions> options = nullopt;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_001";
}

/**
 * @tc.name: CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_002
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreMockTest, CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_002";

    string path = "fakePath/CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_002.txt";
    int32_t mode = 0;
    RandomAccessFileOptions opts;
    opts.start = 0;
    opts.end = 100;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, opts);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsRandomAccessFile> raf(res.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(raf, nullptr);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreMockTest_DoCreateRandomAccessFile_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test