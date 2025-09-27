/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
class CloseCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloseCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CloseCoreMockTest");
    UvFsMock::EnableMock();
}

void CloseCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloseCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void CloseCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t EXPECTED_FD = 1;

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
        .WillOnce(testing::Return(UV_EBADF));
    // Do testing
    auto ret = CloseCore::DoClose(fd);
    // Verify results
    EXPECT_FALSE(ret.IsSuccess());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());

    GTEST_LOG_(INFO) << "CloseCoreMockTest-end CloseCoreMockTest_DoClose_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS