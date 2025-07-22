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

#include "atomicfile_mock.h"
#include <fcntl.h>
#include <gtest/gtest.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

static const string g_atomicfilePath = "/data/test/AtomicfileMockTest.txt";

class AtomicfileMockTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        AtomicfileMock::EnableMock();
        int32_t fd = open(g_atomicfilePath.c_str(), O_CREAT | O_RDWR, 0644);
        close(fd);
    };
    static void TearDownTestCase()
    {
        AtomicfileMock::DisableMock();
        rmdir(g_atomicfilePath.c_str());
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: AtomicfileFailWrite_0001
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFailWrite_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFailWrite_0001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, remove(testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));

    auto res = AtomicFileNExporter::FailWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFailWrite_0001";
}

/**
 * @tc.name: AtomicfileFailWrite_0002
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFailWrite_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFailWrite_0002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, remove(testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, ThrowErr(testing::_, testing::_));

    auto res = AtomicFileNExporter::FailWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFailWrite_0002";
}

/**
 * @tc.name: AtomicfileFailWrite_0003
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFailWrite_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFailWrite_0003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, remove(testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*mock_, ThrowErr(testing::_, testing::_));

    auto res = AtomicFileNExporter::FailWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFailWrite_0003";
}

/**
 * @tc.name: AtomicfileFinishWrite_0001
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFinishWrite_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFinishWrite_0001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, rename(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));

    auto res = AtomicFileNExporter::FinishWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFinishWrite_0001";
}

/**
 * @tc.name: AtomicfileFinishWrite_0002
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFinishWrite_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFinishWrite_0002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, rename(testing::_, testing::_)).WillOnce(testing::Return(-1));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, ThrowErr(testing::_, testing::_));

    auto res = AtomicFileNExporter::FinishWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFinishWrite_0002";
}

/**
 * @tc.name: AtomicfileFinishWrite_0003
 * @tc.desc: Test function of FailWrite() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AtomicfileMockTest, AtomicfileFinishWrite_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AtomicfileMockTest-begin AtomicfileFinishWrite_0003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    AtomicFileEntity entity;
    std::tuple<AtomicFileEntity *, int32_t> tmp(&entity, 1);

    auto mock_ = AtomicfileMock::GetMock();
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&entity)));
    EXPECT_CALL(*mock_, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_typeof(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(napi_undefined), testing::Return(napi_ok)));

    EXPECT_CALL(*mock_, rename(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));
    EXPECT_CALL(*mock_, napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*mock_, ThrowErr(testing::_, testing::_));

    auto res = AtomicFileNExporter::FinishWrite(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "AtomicfileMockTest-end AtomicfileFinishWrite_0003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS