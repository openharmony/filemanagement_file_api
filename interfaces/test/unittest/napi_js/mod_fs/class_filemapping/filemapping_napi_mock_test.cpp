/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "napi/filemapping_napi.h"

#include <cstring>
#include <fcntl.h>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fs_filemapping.h"
#include "libn_mock.h"
#include "mmap_mock.h"
#include "securec.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace testing;
using namespace OHOS::FileManagement::ModuleFileIO;

class FileMappingNapiMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void FileMappingNapiMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FileMappingNapiMockTest");
    LibnMock::EnableMock();
    MmapMock::EnableMock();
}

void FileMappingNapiMockTest::TearDownTestSuite(void)
{
    LibnMock::DisableMock();
    MmapMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FileMappingNapiMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileMappingNapiMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileMappingNapiMockTest_SetPosition_001
 * @tc.desc: Test function of FileMappingNapi::SetPosition interface for FAILURE when argument count is not 1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_SetPosition_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_SetPosition_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::SetPosition(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_SetPosition_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_GetPosition_001
 * @tc.desc: Test function of FileMappingNapi::GetPosition interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_GetPosition_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_GetPosition_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::GetPosition(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_GetPosition_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Capacity_001
 * @tc.desc: Test function of FileMappingNapi::Capacity interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Capacity_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Capacity_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Capacity(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Capacity_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_SetLimit_001
 * @tc.desc: Test function of FileMappingNapi::SetLimit interface for FAILURE when argument count is not 1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_SetLimit_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_SetLimit_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::SetLimit(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_SetLimit_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_GetLimit_001
 * @tc.desc: Test function of FileMappingNapi::GetLimit interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_GetLimit_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_GetLimit_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::GetLimit(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_GetLimit_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Flip_001
 * @tc.desc: Test function of FileMappingNapi::Flip interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Flip_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Flip_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Flip(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Flip_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Remaining_001
 * @tc.desc: Test function of FileMappingNapi::Remaining interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Remaining_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Remaining_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Remaining(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Remaining_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Read_001
 * @tc.desc: Test function of FileMappingNapi::Read interface for FAILURE when argument count is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Read_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Read_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Read(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Read_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Write_001
 * @tc.desc: Test function of FileMappingNapi::Write interface for FAILURE when argument count is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Write_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Write_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Write(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Write_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Msync_001
 * @tc.desc: Test function of FileMappingNapi::Msync interface for FAILURE when argument count is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Msync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Msync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO, NARG_CNT::TWO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Msync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Msync_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_MsyncSync_001
 * @tc.desc: Test function of FileMappingNapi::MsyncSync interface for FAILURE when argument count is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_MsyncSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_MsyncSync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO, NARG_CNT::TWO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::MsyncSync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_MsyncSync_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Unmap_001
 * @tc.desc: Test function of FileMappingNapi::Unmap interface for FAILURE when argument count is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Unmap_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Unmap_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Unmap(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Unmap_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_UnmapSync_001
 * @tc.desc: Test function of FileMappingNapi::UnmapSync interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_UnmapSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_UnmapSync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::UnmapSync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_UnmapSync_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_Constructor_001
 * @tc.desc: Test function of FileMappingNapi::Constructor interface for FAILURE when argument count is not 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Constructor_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = FileMappingNapi::Constructor(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Constructor_001";
}

/**
 * @tc.name: FileMappingNapiMockTest_SetPosition_002
 * @tc.desc: Test function of FileMappingNapi::SetPosition interface for FAILURE when position is negative.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_SetPosition_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_SetPosition_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    auto mmapMock = MmapMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(reinterpret_cast<napi_value>(0x2000)));
    EXPECT_CALL(*libnMock, napi_unwrap(env, reinterpret_cast<napi_value>(0x2000), _))
        .WillOnce(Invoke([](napi_env, napi_value, void **result) {
            *result = nullptr;
            return napi_ok;
        }));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(testing::AtLeast(1));

    auto res = FileMappingNapi::SetPosition(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_SetPosition_002";
}

/**
 * @tc.name: FileMappingNapiMockTest_SetLimit_002
 * @tc.desc: Test function of FileMappingNapi::SetLimit interface for FAILURE when GetMapping returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_SetLimit_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_SetLimit_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(reinterpret_cast<napi_value>(0x2000)));
    EXPECT_CALL(*libnMock, napi_unwrap(env, reinterpret_cast<napi_value>(0x2000), _))
        .WillOnce(Invoke([](napi_env, napi_value, void **result) {
            *result = nullptr;
            return napi_ok;
        }));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(testing::AtLeast(1));

    auto res = FileMappingNapi::SetLimit(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_SetLimit_002";
}

/**
 * @tc.name: FileMappingNapiMockTest_GetPosition_002
 * @tc.desc: Test function of FileMappingNapi::GetPosition interface for FAILURE when GetMapping returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_GetPosition_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_GetPosition_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(reinterpret_cast<napi_value>(0x2000)));
    EXPECT_CALL(*libnMock, napi_unwrap(env, reinterpret_cast<napi_value>(0x2000), _))
        .WillOnce(Invoke([](napi_env, napi_value, void **result) {
            *result = nullptr;
            return napi_ok;
        }));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(testing::AtLeast(1));

    auto res = FileMappingNapi::GetPosition(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_GetPosition_002";
}

/**
 * @tc.name: FileMappingNapiMockTest_Capacity_002
 * @tc.desc: Test function of FileMappingNapi::Capacity interface for FAILURE when GetMapping returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMappingNapiMockTest, FileMappingNapiMockTest_Capacity_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-begin FileMappingNapiMockTest_Capacity_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ZERO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(reinterpret_cast<napi_value>(0x2000)));
    EXPECT_CALL(*libnMock, napi_unwrap(env, reinterpret_cast<napi_value>(0x2000), _))
        .WillOnce(Invoke([](napi_env, napi_value, void **result) {
            *result = nullptr;
            return napi_ok;
        }));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(testing::AtLeast(1));

    auto res = FileMappingNapi::Capacity(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FileMappingNapiMockTest-end FileMappingNapiMockTest_Capacity_002";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
