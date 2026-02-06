/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fsync.h"

#include <fcntl.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;

class FsyncMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void FsyncMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsyncMockTest");
}

void FsyncMockTest::TearDownTestSuite(void)
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsyncMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
    errno = 0;
}

void FsyncMockTest::TearDown(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsyncMockTest_Sync_001
 * @tc.desc: Test function of Fsync::Sync interface for FAILED with ARGS ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncMockTest, FsyncMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncMockTest-begin FsyncMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Fsync::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FsyncMockTest-end FsyncMockTest_Sync_001";
}

/**
 * @tc.name: FsyncMockTest_Sync_002
 * @tc.desc: Test function of Fsync::Sync interface for FAILED with Analyze args ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncMockTest, FsyncMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncMockTest-begin FsyncMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    tuple<bool, int> isFd = { false, 0 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Fsync::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FsyncMockTest-end FsyncMockTest_Sync_002";
}

/**
 * @tc.name: FsyncMockTest_Sync_003
 * @tc.desc: Test function of Fsync::Sync interface for FAILED with uv_fs_fsync ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncMockTest, FsyncMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncMockTest-begin FsyncMockTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filePath = "/data/test/FsyncMockTest_Sync_003.txt";
    char uvArr[] = "FsyncMockTest_Sync_003";
    char *uvPtr = uvArr;
    tuple<bool, int> isFd = { true, 0 };

    auto libnMock = LibnMock::GetMock();
    auto uvFsMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvFsMock, uv_fs_fsync(testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::DoAll(testing::Invoke([uvPtr](uv_loop_t *lop, uv_fs_t *req, uv_file fd, uv_fs_cb cb) {
                req->ptr = static_cast<void *>(uvPtr);
            }), testing::Return(-1)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Fsync::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvFsMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "FsyncMockTest-end FsyncMockTest_Sync_003";
}

/**
 * @tc.name: FsyncMockTest_Sync_004
 * @tc.desc: Test function of Fsync::Sync interface for SUCCEED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncMockTest, FsyncMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncMockTest-begin FsyncMockTest_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x2000);
    string filePath = "/data/test/FsyncMockTest_Sync_004.txt";
    char uvArr[] = "FsyncMockTest_Sync_004";
    char *uvPtr = uvArr;
    tuple<bool, int> isFd = { true, 0 };

    auto libnMock = LibnMock::GetMock();
    auto uvFsMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvFsMock, uv_fs_fsync(testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::DoAll(testing::Invoke([uvPtr](uv_loop_t *lop, uv_fs_t *req, uv_file fd, uv_fs_cb cb) {
                req->ptr = static_cast<void *>(uvPtr);
            }), testing::Return(0)));
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(NVal {env, val}));

    auto res = Fsync::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvFsMock.get());
    EXPECT_EQ(res, val);

    GTEST_LOG_(INFO) << "FsyncMockTest-end FsyncMockTest_Sync_004";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test