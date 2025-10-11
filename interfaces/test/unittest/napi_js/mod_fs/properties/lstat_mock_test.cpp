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

#include "lstat.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <securec.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class LstatMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LstatMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "LstatMockTest");
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
}

void LstatMockTest::TearDownTestCase(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void LstatMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LstatMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatMockTest_Sync_001
 * @tc.desc: Test function of Lstat::Sync interface with URI parameter for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);
    const char *testUri = "file://com.example.statsupporturi/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));
    EXPECT_CALL(*uvMock, uv_fs_lstat(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto stat = Lstat::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Sync_001";
}

/**
 * @tc.name: LstatMockTest_Sync_002
 * @tc.desc: Test function of Lstat::Sync interface with sandbox path for FAILURE when uv_fs_lstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);
    const char *testUri = "/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
        tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));
    EXPECT_CALL(*uvMock, uv_fs_lstat(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto stat = Lstat::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(stat, nullptr);
    
    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Sync_002";
}

/**
 * @tc.name: LstatMockTest_Sync_003
 * @tc.desc: Test function of Lstat::Sync interface with unsupported URI path to verify failure handling.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);

    const char *testUri = "datashare://com.example.statsupporturi/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));

    auto stat = Lstat::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Sync_003";
}

/**
 * @tc.name: LstatMockTest_Async_001
 * @tc.desc: Test function of Lstat::Async interface with URI parameter for FAILURE when async call fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Async_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Async_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);
    const char *testUri = "file://com.example.statsupporturi/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));

    auto stat = Lstat::Async(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Async_001";
}

/**
 * @tc.name: LstatMockTest_Async_002
 * @tc.desc: Test function of Lstat::Async interface with sandbox path for FAILURE when async call fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Async_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Async_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);
    const char *testUri = "/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));

    auto stat = Lstat::Async(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(stat, nullptr);
    
    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Async_002";
}

/**
 * @tc.name: LstatMockTest_Async_003
 * @tc.desc: Test function of Lstat::Async interface with unsupported URI path for FAILURE when unsupported URI is provided.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatMockTest, LstatMockTest_Async_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatMockTest-begin LstatMockTest_Async_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);

    const char *testUri = "datashare://com.example.statsupporturi/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testUri) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testUri) + 1, testUri, strlen(testUri));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));

    auto stat = Lstat::Async(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "LstatMockTest-end LstatMockTest_Async_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS