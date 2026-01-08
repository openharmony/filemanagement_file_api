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

#include "open_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "unistd_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class OpenCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void OpenCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "OpenCoreMockTest");
    UvFsMock::EnableMock();
    UnistdMock::EnableMock();
}

void OpenCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void OpenCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void OpenCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_001
 * @tc.desc: Test function of OpenCore::DoOpen interface for SUCCESS when path is local file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_001";

    string path = "fakePath/OpenCoreMockTest_DoOpen_001.txt";
    int32_t mode = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));

    auto res = OpenCore::DoOpen(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsFile> file(res.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_001";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_002
 * @tc.desc: Test function of OpenCore::DoOpen interface for SUCCESS when path is docs URL.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_002";

    string path = "file://docs/fakePath/OpenCoreMockTest_DoOpen_002.txt";
    int32_t mode = 0;

    auto uvMock = UvFsMock::GetMock();
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).WillRepeatedly(testing::Return(0));

    auto res = OpenCore::DoOpen(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsFile> file(res.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_002";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_003
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when path is docs URL and uv_fs_open fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_003";

    string path = "file://docs/fakePath/OpenCoreMockTest_DoOpen_003.txt";
    int32_t mode = 0;

    auto uvMock = UvFsMock::GetMock();
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = OpenCore::DoOpen(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_003";
}

/**
 * @tc.name: OpenCoreMockTest_DoOpen_004
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when path is a local file and uv_fs_open fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreMockTest, OpenCoreMockTest_DoOpen_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreMockTest-begin OpenCoreMockTest_DoOpen_004";

    string path = "fakePath/OpenCoreMockTest_DoOpen_004";
    int32_t mode = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = OpenCore::DoOpen(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "OpenCoreMockTest-end OpenCoreMockTest_DoOpen_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test