/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "atomicfile_n_exporter.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "atomicfile_entity.h"
#include "libn_mock.h"
#include "stdio_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class AtomicfileMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AtomicfileMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "AtomicfileMockTest");
    LibnMock::EnableMock();
    StdioMock::EnableMock();
}

void AtomicfileMockTest::TearDownTestCase()
{
    LibnMock::DisableMock();
    StdioMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AtomicfileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0;
}

void AtomicfileMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

static const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

/**
 * @tc.name: AtomicfileMockTest_FailWrite_001
 * @tc.desc: Test function of Atomicfile::FailWrite interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileMockTest_FailWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileMockTest_FailWrite_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filepath = "fakePath/AtomicfileMockTest_FailWrite_001";
    AtomicFileEntity entity;
    entity.baseFileName = filepath;
    entity.newFileName = filepath.append(TEMP_FILE_SUFFIX);
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto libnMock = LibnMock::GetMock();
    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*stdioMock, remove(testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));

    auto res = AtomicFileNExporter::FailWrite(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileMockTest_FailWrite_001";
}

/**
 * @tc.name: AtomicfileMockTest_FailWrite_002
 * @tc.desc: Test function of Atomicfile::FailWrite interface for FAILURE when napi_delete_reference fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileMockTest_FailWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileMockTest_FailWrite_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filepath = "fakePath/AtomicfileMockTest_FailWrite_002";
    AtomicFileEntity entity;
    entity.baseFileName = filepath;
    entity.newFileName = filepath.append(TEMP_FILE_SUFFIX);
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto libnMock = LibnMock::GetMock();
    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*stdioMock, remove(testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_));

    auto res = AtomicFileNExporter::FailWrite(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileMockTest_FailWrite_002";
}

/**
 * @tc.name: AtomicfileMockTest_FinishWrite_001
 * @tc.desc: Test function of Atomicfile::FinishWrite interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileMockTest_FinishWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileMockTest_FinishWrite_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filepath = "fakePath/AtomicfileMockTest_FinishWrite_001";
    AtomicFileEntity entity;
    entity.baseFileName = filepath;
    entity.newFileName = filepath.append(TEMP_FILE_SUFFIX);
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto libnMock = LibnMock::GetMock();
    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*stdioMock, rename(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));

    auto res = AtomicFileNExporter::FinishWrite(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileMockTest_FinishWrite_001";
}

/**
 * @tc.name: AtomicfileMockTest_FinishWrite_002
 * @tc.desc: Test function of Atomicfile::FinishWrite interface for FAILURE when rename fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileMockTest_FinishWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileMockTest_FinishWrite_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filepath = "fakePath/AtomicfileMockTest_FinishWrite_002";
    AtomicFileEntity entity;
    entity.baseFileName = filepath;
    entity.newFileName = filepath.append(TEMP_FILE_SUFFIX);
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto libnMock = LibnMock::GetMock();
    auto stdioMock = StdioMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*stdioMock, rename(testing::_, testing::_)).WillOnce(testing::Return(1));
    EXPECT_CALL(*libnMock, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_));

    auto res = AtomicFileNExporter::FinishWrite(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileMockTest_FinishWrite_002";
}

/**
 * @tc.name: AtomicfileMockTest_FinishWrite_003
 * @tc.desc: Test function of Atomicfile::FinishWrite interface for FAILURE when napi_delete_reference fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileMockTest_FinishWrite_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileMockTest_FinishWrite_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    string filepath = "fakePath/AtomicfileMockTest_FinishWrite_003";
    AtomicFileEntity entity;
    entity.baseFileName = filepath;
    entity.newFileName = filepath.append(TEMP_FILE_SUFFIX);
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto libnMock = LibnMock::GetMock();
    auto stdioMock = StdioMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*libnMock, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*stdioMock, rename(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_));

    auto res = AtomicFileNExporter::FinishWrite(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileMockTest_FinishWrite_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS