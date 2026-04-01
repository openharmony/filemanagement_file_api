/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "close_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fdtag_func.h"
#include "fdsan_mock.h"
#include "open_core.h"
#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
class CloseCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CloseCoreTest";
};

void CloseCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CloseCoreMockTest");
    FdsanMock::EnableMock();
    UvFsMock::EnableMock();
}

void CloseCoreMockTest::TearDownTestSuite()
{
    FdsanMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CloseCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void CloseCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t EXPECTED_FD = 1;
inline const int32_t EXPECTED_MAX_FD = 2048;

/**
 * @tc.name: CloseCoreMockTest_DoClose_001
 * @tc.desc: Test function of CloseCore::DoClose interface for FAILURE when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreMockTest, CloseCoreMockTest_DoClose_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreMockTest-begin CloseCoreMockTest_DoClose_001";

    // Prepare test parameters
    int fd = EXPECTED_FD;
    // Set mock behaviors
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(-EBADF));
    // Do testing
    auto ret = CloseCore::DoClose(fd);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900008);
    EXPECT_EQ(err.GetErrMsg(), "Bad file descriptor");

    GTEST_LOG_(INFO) << "CloseCoreMockTest-end CloseCoreMockTest_DoClose_001";
}

/**
 * @tc.name: CloseCoreMockTest_DoClose_002
 * @tc.desc: Test function of CloseCore::DoClose interface for FAILURE when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreMockTest, CloseCoreMockTest_DoClose_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreMockTest-begin CloseCoreMockTest_DoClose_002";
 
    // Prepare test parameters
    int fd = EXPECTED_MAX_FD;
    // Set mock behaviors
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(-EBADF));
    // Do testing
    auto ret = CloseCore::DoClose(fd);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900008);
    EXPECT_EQ(err.GetErrMsg(), "Bad file descriptor");
 
    GTEST_LOG_(INFO) << "CloseCoreMockTest-end CloseCoreMockTest_DoClose_002";
}
 
/**
 * @tc.name: CloseCoreMockTest_DoClose_003
 * @tc.desc: Test function of CloseCore::DoClose(file) interface for FAILURE when uv_fs_close fails
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreMockTest, CloseCoreMockTest_DoClose_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreMockTest-begin CloseCoreMockTest_DoClose_003";
 
    auto path = testDir + "/CloseCoreMockTest_DoClose_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "CloseCoreMockTest_DoClose_003"));
 
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADFD, -1));
 
    auto fileRes = OpenCore::DoOpen(path);
    ASSERT_TRUE(fileRes.IsSuccess());
    std::unique_ptr<FsFile> file(fileRes.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);
 
    auto ret = CloseCore::DoClose(file.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_FALSE(ret.IsSuccess());
 
    GTEST_LOG_(INFO) << "CloseCoreMockTest-end CloseCoreMockTest_DoClose_003";
}
} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS