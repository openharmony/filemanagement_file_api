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

#include "napi/mmap_napi.h"

#include <cstring>
#include <fcntl.h>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

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

class MmapNapiMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void MmapNapiMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MmapNapiMockTest");
    LibnMock::EnableMock();
    MmapMock::EnableMock();
}

void MmapNapiMockTest::TearDownTestSuite(void)
{
    LibnMock::DisableMock();
    MmapMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MmapNapiMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MmapNapiMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_001
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when argument count is not 4.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_001";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_002
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when fd < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, -1)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64())
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))))
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(1024))));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_002";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_003
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_003";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(false, 100)));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_003";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_004
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when offset is negative.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_004";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(false, static_cast<int64_t>(-1))));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_004";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_005
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when size is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_005";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64())
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))))
        .WillOnce(Return(make_tuple(false, static_cast<int64_t>(0))));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_005";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_006
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when DoMmap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_006";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    auto mmapMock = MmapMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64())
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))))
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(1024))));
    
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    EXPECT_CALL(*mmapMock, fstat(10, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(10, _)).WillOnce(Return(-1));
    
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_006";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_007
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when argument count is less than 4.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_007";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(1));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_007";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_008
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when mode < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_008";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, -1)));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_008";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_009
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when mode > MappingMode::PRIVATE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_009";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 100)));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_009";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_010
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_010";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(true, static_cast<int64_t>(-1))));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_010";
}

/**
 * @tc.name: MmapNapiMockTest_Sync_011
 * @tc.desc: Test function of MmapNapi::Sync interface for FAILURE when size is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapNapiMockTest, MmapNapiMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapNapiMockTest-begin MmapNapiMockTest_Sync_011";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::FOUR)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(Return(4));
    EXPECT_CALL(*libnMock, ToInt32())
        .WillOnce(Return(make_tuple(true, 10)))
        .WillOnce(Return(make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ToInt64())
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))))
        .WillOnce(Return(make_tuple(true, static_cast<int64_t>(-1))));
    EXPECT_CALL(*libnMock, ThrowErr(env)).Times(1);

    auto res = MmapNapi::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MmapNapiMockTest-end MmapNapiMockTest_Sync_011";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
