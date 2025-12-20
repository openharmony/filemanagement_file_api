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
 * @tc.desc: Test function of Close::Sync interface if is fd for FAILURE when fdsan_close_with_tag fails.
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
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADFD, -1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
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
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, int> isFd = { true, 1 };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*fdsanMock, fdsan_get_owner_tag(testing::_)).WillOnce(testing::Return(1));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

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
    auto uvMock = UvFsMock::GetMock();
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
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
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
    auto uvMock = UvFsMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillOnce(testing::Return(nv)).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(1));

    auto res = Close::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CloseMockTest-end CloseMockTest_Sync_004";
}

/**
 * @tc.name: GetFdTag_001
 * @tc.desc: Test function of CommonFunc::GetFdTag interface if is fd for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, GetFdTag_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin GetFdTag_001";

    uint64_t res = CommonFunc::GetFdTag(-1);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end GetFdTag_001";
}

/**
 * @tc.name: GetFdTag_002
 * @tc.desc: Test function of CommonFunc::GetFdTag interface if is fd for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, GetFdTag_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin GetFdTag_002";

    uint64_t res = CommonFunc::GetFdTag(2049);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end GetFdTag_002";
}

/**
 * @tc.name: SetFdTag_001
 * @tc.desc: Test function of CommonFunc::SetFdTag interface if is fd for FAILURE when GetFdSanEntry fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CommonFuncTest, SetFdTag_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommonFuncTest-begin SetFdTag_001";

    CommonFunc::SetFdTag(-1, 0);
    uint64_t res = CommonFunc::GetFdTag(-1);

    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CommonFuncTest-end SetFdTag_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS