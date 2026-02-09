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

#include "dup.h"

#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;

static const int INVALID_FD = 65536;

class DupMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void DupMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "DupMockTest");
}

void DupMockTest::TearDownTestSuite(void)
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void DupMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
    errno = 0;
}

void DupMockTest::TearDown(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DupMockTest_Sync_001
 * @tc.desc: Test function of Dup::Sync interface for FAILED with ARGS ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupMockTest, DupMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupMockTest-begin DupMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Dup::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "DupMockTest-end DupMockTest_Sync_001";
}

/**
 * @tc.name: DupMockTest_Sync_002
 * @tc.desc: Test function of Dup::Sync interface for FAILED with ToInt32 ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupMockTest, DupMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupMockTest-begin DupMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    tuple<bool, int> isFd = { false, 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Dup::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "DupMockTest-end DupMockTest_Sync_002";
}

/**
 * @tc.name: DupMockTest_Sync_003
 * @tc.desc: Test function of Dup::Sync interface for FAILED with dup ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupMockTest, DupMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupMockTest-begin DupMockTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    tuple<bool, int> isFd = { true, INVALID_FD };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Dup::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "DupMockTest-end DupMockTest_Sync_003";
}

/**
 * @tc.name: DupMockTest_Sync_004
 * @tc.desc: Test function of Dup::Sync interface for FAILED with uv_fs_readlink ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupMockTest, DupMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupMockTest-begin DupMockTest_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filePath = "/data/test/DupMockTest_Sync_004.txt";
    int srcFd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    EXPECT_GT(srcFd, -1);
    tuple<bool, int> isFd = { true, srcFd };

    auto libnMock = LibnMock::GetMock();
    auto uvFsMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvFsMock, uv_fs_readlink(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Dup::Sync(env, info);
    close(srcFd);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvFsMock.get());
    EXPECT_TRUE(filesystem::remove(filePath));
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "DupMockTest-end DupMockTest_Sync_004";
}

/**
* @tc.name: DupMockTest_Sync_005
* @tc.desc: Test function of Dup::Sync interface for SUCCEED.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
*/
HWTEST_F(DupMockTest, DupMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DupMockTest-begin DupMockTest_Sync_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filePath = "/data/test/DupMockTest_Sync_005.txt";
    char uvArr[] = "DupMockTest_Sync_005";
    char *uvPtr = uvArr;
    int srcFd = open(filePath.c_str(), O_CREAT|O_RDWR, 0644);
    EXPECT_GT(srcFd, -1);
    tuple<bool, int> isFd = { true, srcFd };

    auto libnMock = LibnMock::GetMock();
    auto uvFsMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(isFd));
    EXPECT_CALL(*uvFsMock, uv_fs_readlink(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::Invoke([uvPtr](uv_loop_t *lop, uv_fs_t *req, const char *path, uv_fs_cb cb) {
            req->ptr = static_cast<void *>(uvPtr);
        }), testing::Return(0)));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Dup::Sync(env, info);
    close(srcFd);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvFsMock.get());
    EXPECT_TRUE(filesystem::remove(filePath));
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "DupMockTest-end DupMockTest_Sync_005";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test