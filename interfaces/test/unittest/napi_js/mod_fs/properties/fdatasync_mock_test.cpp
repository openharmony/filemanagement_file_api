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

#include "fdatasync.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FdatasyncMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FdatasyncMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "FdatasyncMockTest");
    UvFsMock::EnableMock();
    LibnMock::EnableMock();
}

void FdatasyncMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FdatasyncMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FdatasyncMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FdatasyncMockTest_Sync_001
 * @tc.desc: Test function of Fdatasync::Sync interface for FAILURE when uv_fs_fdatasync fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FdatasyncMockTest, FdatasyncMockTest_Sync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdatasyncMockTest-begin FdatasyncMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    tuple<bool, int32_t> tp = { true, 1 };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(Return(tp));
    EXPECT_CALL(*uvMock, uv_fs_fdatasync(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Fdatasync::Sync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FdatasyncMockTest-end FdatasyncMockTest_Sync_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test