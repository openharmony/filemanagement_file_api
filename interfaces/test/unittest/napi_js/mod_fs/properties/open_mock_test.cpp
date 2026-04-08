/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "open.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <securec.h>
#include <sys/prctl.h>

#include "class_file/file_entity.h"
#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;
using namespace OHOS::FileManagement::LibN;

class OpenMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void OpenMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "OpenMockTest");
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
}

void OpenMockTest::TearDownTestSuite(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void OpenMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void OpenMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OpenMockTest_Sync_001
 * @tc.desc: Test function of Open::Sync interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_001";
}

/**
 * @tc.name: OpenMockTest_Sync_002
 * @tc.desc: Test function of Open::Sync interface for FAILURE when ToUTF8StringPath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    tuple<bool, unique_ptr<char[]>, size_t> pathResult = { false, nullptr, 0 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_002";
}

/**
 * @tc.name: OpenMockTest_Sync_003
 * @tc.desc: Test function of Open::Sync interface for FAILURE when uv_fs_open fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *testPath = "/data/test/OpenMockTest_Sync_003.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_003";
}

/**
 * @tc.name: OpenMockTest_Sync_004
 * @tc.desc: Test function of Open::Sync interface for FAILURE when InstantiateClass fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *testPath = "/data/test/OpenMockTest_Sync_004.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_004";
}

/**
 * @tc.name: OpenMockTest_Sync_005
 * @tc.desc: Test function of Open::Sync interface for FAILURE when GetJsFlags fails with invalid mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *testPath = "/data/test/OpenMockTest_Sync_005.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    tuple<bool, int32_t> modeResult = { true, -1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(2));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(modeResult));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_005";
}

/**
 * @tc.name: OpenMockTest_Sync_006
 * @tc.desc: Test function of Open::Sync interface for SUCCESS with default mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_006";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value objFile = reinterpret_cast<napi_value>(0x1200);

    const char *testPath = "/data/test/OpenMockTest_Sync_006.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(10);

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(objFile));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_006";
}

/**
 * @tc.name: OpenMockTest_Sync_007
 * @tc.desc: Test function of Open::Sync interface for SUCCESS with custom mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_007";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value objFile = reinterpret_cast<napi_value>(0x1200);

    const char *testPath = "/data/test/OpenMockTest_Sync_007.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    tuple<bool, int32_t> modeResult = { true, O_WRONLY };

    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(10);

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(2));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(modeResult));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(objFile));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_007";
}

/**
 * @tc.name: OpenMockTest_Sync_008
 * @tc.desc: Test function of Open::Sync interface for FAILURE when GetEntityOf fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_008";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value objFile = reinterpret_cast<napi_value>(0x1200);

    const char *testPath = "/data/test/OpenMockTest_Sync_008.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(objFile));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_008";
}

/**
 * @tc.name: OpenMockTest_Sync_009
 * @tc.desc: Test function of Open::Sync interface for FAILURE with invalid mode (O_WRONLY | O_RDWR).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_009";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *testPath = "/data/test/OpenMockTest_Sync_009.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    tuple<bool, int32_t> modeResult = { true, O_WRONLY | O_RDWR };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(2));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(modeResult));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_009";
}

/**
 * @tc.name: OpenMockTest_Sync_010
 * @tc.desc: Test function of Open::Sync interface for SUCCESS with O_RDWR mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_010";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value objFile = reinterpret_cast<napi_value>(0x1200);

    const char *testPath = "/data/test/OpenMockTest_Sync_010.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    tuple<bool, int32_t> modeResult = { true, O_RDWR };

    FileEntity entity;
    entity.fd_ = std::make_unique<DistributedFS::FDGuard>(10);

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(2));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(modeResult));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_open(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(objFile));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_010";
}

/**
 * @tc.name: OpenMockTest_Sync_011
 * @tc.desc: Test function of Open::Sync interface for FAILURE when ToInt32 returns false for mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenMockTest, OpenMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenMockTest-begin OpenMockTest_Sync_011";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *testPath = "/data/test/OpenMockTest_Sync_011.txt";
    tuple<bool, unique_ptr<char[]>, size_t> pathResult = {
        true,
        [&]() -> unique_ptr<char[]> {
            auto ptr = make_unique<char[]>(strlen(testPath) + 1);
            auto ret = strncpy_s(ptr.get(), strlen(testPath) + 1, testPath, strlen(testPath));
            EXPECT_EQ(ret, EOK);
            return ptr;
        }(),
        1
    };

    tuple<bool, int32_t> modeResult = { false, 0 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(std::move(pathResult))));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(2));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(modeResult));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Open::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "OpenMockTest-end OpenMockTest_Sync_011";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
