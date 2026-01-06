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

#include "access_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class AccessCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void AccessCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "AccessCoreMockTest");
    UvFsMock::EnableMock();
}

void AccessCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void AccessCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AccessCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AccessCoreMockTest_DoAccess_001
 * @tc.desc: Test function of AccessCore::ValidAccessArgs interface for FAILURE when uv_fs_access fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreMockTest, AccessCoreMockTest_DoAccess_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreMockTest-begin AccessCoreMockTest_DoAccess_001";

    std::string path = "fakePath/AccessCoreMockTest_DoAccess_001.txt";
    std::optional<AccessModeType> mode;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = AccessCore::DoAccess(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "AccessCoreMockTest-end AccessCoreMockTest_DoAccess_001";
}

/**
 * @tc.name: AccessCoreMockTest_DoAccess_002
 * @tc.desc: Test function of AccessCore::ValidAccessArgs interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreMockTest, AccessCoreMockTest_DoAccess_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreMockTest-begin AccessCoreMockTest_DoAccess_002";

    std::string path = "fakePath/AccessCoreMockTest_DoAccess_002.txt";
    std::optional<AccessModeType> mode;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));

    auto res = AccessCore::DoAccess(path, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_TRUE(exists);

    GTEST_LOG_(INFO) << "AccessCoreMockTest-end AccessCoreMockTest_DoAccess_002";
}

/**
 * @tc.name: AccessCoreMockTest_DoAccess_003
 * @tc.desc: Test function of AccessCore::DoAccess interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreMockTest, AccessCoreMockTest_DoAccess_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreMockTest-begin AccessCoreMockTest_DoAccess_003";

    std::string path = "fakePath/AccessCoreMockTest_DoAccess_003.txt";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = DEFAULT_FLAG;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(-ENOENT));

    auto res = AccessCore::DoAccess(path, mode, flag);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_FALSE(exists);

    GTEST_LOG_(INFO) << "AccessCoreMockTest-end AccessCoreMockTest_DoAccess_003";
}

/**
 * @tc.name: AccessCoreMockTest_DoAccess_004
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreMockTest, AccessCoreMockTest_DoAccess_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreMockTest-begin AccessCoreMockTest_DoAccess_004";

    std::string path = "fakePath/AccessCoreMockTest_DoAccess_004.txt";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = DEFAULT_FLAG;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));

    auto res = AccessCore::DoAccess(path, mode, flag);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_TRUE(exists);

    GTEST_LOG_(INFO) << "AccessCoreMockTest-end AccessCoreMockTest_DoAccess_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test