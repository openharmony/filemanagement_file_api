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

#include "dup_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "unistd_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DupCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DupCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "DupCoreMockTest");
    UvFsMock::EnableMock();
    UnistdMock::EnableMock();
}

void DupCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DupCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DupCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DupCoreMockTest_DoDup_001
 * @tc.desc: Test function of DupCore::DoDup interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupCoreMockTest, DupCoreMockTest_DoDup_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupCoreMockTest-begin DupCoreMockTest_DoDup_001";

    int32_t fd = 1;

    auto unistdMock = UnistdMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_readlink(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*unistdMock, dup(_)).WillOnce(Return(1));

    auto res = DupCore::DoDup(fd);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "DupCoreMockTest-end DupCoreMockTest_DoDup_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test