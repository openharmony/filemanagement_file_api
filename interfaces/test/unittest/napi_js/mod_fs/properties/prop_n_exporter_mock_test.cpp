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

#include "uv_fs_mock.h"

#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "prop_n_exporter.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class PropNExporterMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void PropNExporterMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "PropNExporterMockTest");
    UvFsMock::EnableMock();
    LibnMock::EnableMock();
}

void PropNExporterMockTest::TearDownTestSuite(void)
{
    UvFsMock::DisableMock();
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void PropNExporterMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void PropNExporterMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: PropNExporterMockTest_UnlinkSync_001
 * @tc.desc: Test function of PropNExporter::UnlinkSync interface for FAILURE when uv_fs_unlink fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(PropNExporterMockTest, PropNExporterMockTest_UnlinkSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin PropNExporterMockTest_UnlinkSync_001";
    int envAddr = 0x1000;
    int callbackInfoAddr = 0x1122;
    napi_env env = reinterpret_cast<napi_env>(envAddr);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(callbackInfoAddr);

    size_t strLen = 10;
    auto strPtr = make_unique<char[]>(strLen);
    tuple<bool, std::unique_ptr<char[]>, size_t> tp = { true, move(strPtr), strLen };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(tp)));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = PropNExporter::UnlinkSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end PropNExporterMockTest_UnlinkSync_001";
}

/**
 * @tc.name: AccessTest_Sync_001
 * @tc.desc: Test function of PropNExporter::AccessSync interface for FAILED with ARGS ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(PropNExporterMockTest, AccessTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin AccessTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = PropNExporter::AccessSync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end AccessTest_Sync_001";
}

/**
 * @tc.name: AccessTest_Sync_002
 * @tc.desc: Test function of PropNExporter::AccessSync interface for FAILED with ToUTF8StringPath ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(PropNExporterMockTest, AccessTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin AccessTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    size_t strLen = 10;
    auto strPtr = make_unique<char[]>(strLen);
    tuple<bool, unique_ptr<char[]>, size_t> isStr = { false, move(strPtr), strLen };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(isStr)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = PropNExporter::AccessSync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end AccessTest_Sync_002";
}

/**
 * @tc.name: AccessTest_Sync_003
 * @tc.desc: Test function of PropNExporter::AccessSync interface for FAILED with invalid mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(PropNExporterMockTest, AccessTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin AccessTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    size_t strLen = 10;
    auto strPtr = make_unique<char[]>(strLen);
    tuple<bool, unique_ptr<char[]>, size_t> isStr = { true, move(strPtr), strLen };
    tuple<bool, int> isMode = { true, -1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(isStr)));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isMode));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = PropNExporter::AccessSync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end AccessTest_Sync_003";
}

/**
 * @tc.name: AccessTest_Sync_004
 * @tc.desc: Test function of PropNExporter::AccessSync interface for FAILED with invalid flag.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(PropNExporterMockTest, AccessTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin AccessTest_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    size_t strLen = 10;
    auto strPtr = make_unique<char[]>(strLen);
    tuple<bool, unique_ptr<char[]>, size_t> isStr = { true, move(strPtr), strLen };
    tuple<bool, int> isMode = { true, 0 };
    tuple<bool, int> isFlag = { false, 0 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(isStr)));
    EXPECT_CALL(*libnMock, GetArgc())
        .WillOnce(testing::Return(NARG_CNT::THREE))
        .WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isMode));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(isFlag));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = PropNExporter::AccessSync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end AccessTest_Sync_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test