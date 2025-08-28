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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "close_core.h"
#include "mock/uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class CloseCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<UvfsMock> uvMock = nullptr;
};

void CloseCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CloseCoreMockTest");
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void CloseCoreMockTest::TearDownTestCase(void)
{
    Uvfs::ins = nullptr;
    uvMock = nullptr;
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
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(UV_EBADF));
    auto ret = CloseCore::DoClose(1);
    EXPECT_FALSE(ret.IsSuccess());
    GTEST_LOG_(INFO) << "CloseCoreMockTest-end CloseCoreMockTest_DoClose_001";
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS