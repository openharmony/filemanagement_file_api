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

#include "common_func.h"
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
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void CloseMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CloseMockTest");
    LibnMock::EnableMock();
    FdsanMock::EnableMock();
    UvFsMock::EnableMock();
}

void CloseMockTest::TearDownTestSuite(void)
{
    LibnMock::DisableMock();
    FdsanMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
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
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when uv_fs_close fails.
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
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_001";
}

/**
 * @tc.name: CloseMockTest_Sync_002
 * @tc.desc: Test function of Close::Sync interface if is fd for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    NVal mockNval = { env, nVal };

    tuple<bool, int> isFd = { true, 1 };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_002";
}

/**
 * @tc.name: CloseMockTest_Sync_003
 * @tc.desc: Test function of Close::Sync interface if no fd for FAILURE when fdsan_close_with_tag fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_003";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_003";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADFD, -1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_003";
}

/**
 * @tc.name: CloseMockTest_Sync_004
 * @tc.desc: Test function of Close::Sync interface if no fd for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_004";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_004";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_004";
}

/**
 * @tc.name: CloseMockTest_Sync_005
 * @tc.desc: Test function of Close::Sync interface if no fd for FAILURE when ToInt32 fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { true, -1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_005";
}

/**
 * @tc.name: CloseMockTest_Sync_006
 * @tc.desc: Test function of Close::Sync interface if no fd for FAILURE when GetArg fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_006";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_006";
}

/**
 * @tc.name: CloseMockTest_Sync_007
 * @tc.desc: Test function of Close::Sync interface if is fd for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_007";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    NVal mockNval = { env, nVal };

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_007";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_007";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));
    EXPECT_CALL(*libnMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_007";
}

/**
 * @tc.name: CloseMockTest_Sync_008
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when GetArg fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_008";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_008";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_008";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_008";
}

/**
 * @tc.name: CloseMockTest_Sync_009
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when fdsan_close_with_tag fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_009";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_009";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_009";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));
    EXPECT_CALL(*libnMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)),
        testing::Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_009";
}

/**
 * @tc.name: CloseMockTest_Sync_0010
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when fdsan_close_with_tag fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_0010";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { false, 1 };

    string filepath = "fakePath/CloseMockTest_Sync_0010";
    std::string fakeUri = "distributedfs://fake/CloseMockTest_Sync_0010";
    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(0x100);
    entity.path_ = filepath;
    entity.uri_ = fakeUri;

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)),
        testing::Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_0010";
}

/**
 * @tc.name: CloseMockTest_Sync_0011
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when fdsan_close_with_tag fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_0011";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_0011";
}

/**
 * @tc.name: CloseMockTest_Sync_0012
 * @tc.desc: Test function of Close::Sync interface for FAILURE when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_0012";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { true, FD_SAN_OVERFLOW_END };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_0012";
}

/**
 * @tc.name: CloseMockTest_Sync_0013
 * @tc.desc: Test function of Close::Sync interface for SUCCESS when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseMockTest, CloseMockTest_Sync_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseMockTest-begin CloseMockTest_Sync_0013";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { true, FD_SAN_OVERFLOW_END };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_close(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_0013";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS