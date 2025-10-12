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

#include "stat.h"

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

class StatMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StatMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "StatMockTest");
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
}

void StatMockTest::TearDownTestCase(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void StatMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatMockTest_Sync_001
 * @tc.desc: Test function of Stat::Sync interface with URI parameter for FAILURE when uv_fs_stat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatMockTest, StatMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatMockTest-begin StatMockTest_Sync_001";
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
    EXPECT_CALL(*uvMock, uv_fs_stat(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto stat = Stat::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "StatMockTest-end StatURI_0001";
}

/**
 * @tc.name: StatMockTest_Sync_002
 * @tc.desc: Test function of Stat::Sync interface with sandbox path for FAILURE when uv_fs_stat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatMockTest, StatMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatMockTest-begin StatMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);
    const char *testPath = "/data/storage/el2/base/files/test.txt";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    tuple<bool, unique_ptr<char[]>, size_t> uriPathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(uriPathResult))));
    EXPECT_CALL(*uvMock, uv_fs_stat(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));

    auto stat = Stat::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "StatMockTest-end StatMockTest_Sync_002";
}

/**
 * @tc.name: StatMockTest_Sync_003
 * @tc.desc: Test function of Stat::Sync interface with unsupported URI path for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatMockTest, StatMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatMockTest-begin StatMockTest_Sync_003";
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

    auto stat = Stat::Sync(env, info);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(stat, nullptr);

    GTEST_LOG_(INFO) << "StatMockTest-end StatMockTest_Sync_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS