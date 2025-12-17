/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "close.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fdsan_mock.h"
#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class CloseMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloseMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CloseMockTest");
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
    FdsanMock::EnableMock();
}

void CloseMockTest::TearDownTestCase(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    FdsanMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloseMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloseMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CloseMockTest_Sync_001
 * @tc.desc: Test function of Close::Sync interface for FAILURE when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { true, 1 };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*fdsanMock, fdsan_get_owner_tag(testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS