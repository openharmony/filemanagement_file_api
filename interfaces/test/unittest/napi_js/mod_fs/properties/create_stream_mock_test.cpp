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

#include "create_stream.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <tuple>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "class_stream/stream_entity.h"
#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
namespace {
constexpr size_t NAPI_UNWRAP_RESULT_INDEX = 2;
const napi_env ENV = reinterpret_cast<napi_env>(0x4100);
const napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x4200);
const napi_value STREAM_OBJECT = reinterpret_cast<napi_value>(0x4300);
constexpr char TEST_ROOT[] = "/data/test/CreateStreamMockTest";
constexpr char EXISTING_FILE[] = "/data/test/CreateStreamMockTest/existing.txt";
constexpr char CREATED_FILE[] = "/data/test/CreateStreamMockTest/created.txt";
constexpr char MISSING_FILE[] = "/data/test/CreateStreamMockTest/missing.txt";

using StringResult = std::tuple<bool, std::unique_ptr<char[]>, size_t>;

StringResult MakeStringResult(const char *value, bool success = true)
{
    if (!success) {
        return { false, nullptr, 0 };
    }
    size_t length = std::strlen(value);
    auto buffer = std::make_unique<char[]>(length + 1);
    std::copy_n(value, length + 1, buffer.get());
    return { true, std::move(buffer), length };
}
} // namespace

class CreateStreamMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::error_code errorCode;
        std::filesystem::create_directories(TEST_ROOT, errorCode);
        ASSERT_FALSE(errorCode);
        FILE *file = std::fopen(EXISTING_FILE, "w");
        ASSERT_NE(file, nullptr);
        constexpr char content[] = "file_api create stream mock test";
        EXPECT_EQ(std::fwrite(content, 1, sizeof(content) - 1, file), sizeof(content) - 1);
        EXPECT_EQ(std::fclose(file), 0);
    }

    static void TearDownTestSuite()
    {
        std::error_code errorCode;
        std::filesystem::remove_all(TEST_ROOT, errorCode);
    }

    void SetUp() override
    {
        LibnMock::EnableMock();
        std::error_code errorCode;
        std::filesystem::remove(CREATED_FILE, errorCode);
        std::filesystem::remove(MISSING_FILE, errorCode);
    }

    void TearDown() override
    {
        entity_.fp.reset();
        LibnMock::GetMock()->ResetErrState();
        LibnMock::DisableMock();
    }

    void ExpectArguments(const char *path, const char *mode)
    {
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Return(testing::ByMove(MakeStringResult(path))));
        EXPECT_CALL(*mock, ToUTF8String())
            .WillOnce(testing::Return(testing::ByMove(MakeStringResult(mode))));
    }

    void ExpectStreamInstantiation()
    {
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, InstantiateClass(ENV, testing::_, testing::_)).WillOnce(testing::Return(STREAM_OBJECT));
        EXPECT_CALL(*mock, napi_unwrap(ENV, STREAM_OBJECT, testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<NAPI_UNWRAP_RESULT_INDEX>(&entity_), testing::Return(napi_ok)));
    }

private:
    StreamEntity entity_;
};

/**
 * @tc.name: CreateStreamMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an argument count different from two.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_002
 * @tc.desc: Verify Sync rejects a path value that cannot be converted to UTF-8.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_003
 * @tc.desc: Verify Sync rejects a mode value that cannot be converted to UTF-8.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(EXISTING_FILE))));
    EXPECT_CALL(*mock, ToUTF8String())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_004
 * @tc.desc: Verify read mode opens an existing file and returns a stream object.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "r");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
}

/**
 * @tc.name: CreateStreamMockTest_Sync_005
 * @tc.desc: Verify write mode creates a missing file and owns its FILE handle.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(CREATED_FILE, "w");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
    EXPECT_TRUE(std::filesystem::exists(CREATED_FILE));
}

/**
 * @tc.name: CreateStreamMockTest_Sync_006
 * @tc.desc: Verify append mode creates a missing file and returns the stream.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(CREATED_FILE, "a");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
    EXPECT_TRUE(std::filesystem::is_regular_file(CREATED_FILE));
}

/**
 * @tc.name: CreateStreamMockTest_Sync_007
 * @tc.desc: Verify update mode opens an existing file for reading and writing.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "r+");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
}

/**
 * @tc.name: CreateStreamMockTest_Sync_008
 * @tc.desc: Verify a missing file in read mode reports the filesystem error.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(MISSING_FILE, "r");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_009
 * @tc.desc: Verify an unsupported fopen mode reports invalid argument.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "not-a-mode");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_010
 * @tc.desc: Verify stream-constructor failure closes the file and reports I/O error.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "r");
    EXPECT_CALL(*mock, InstantiateClass(ENV, testing::_, testing::_)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900005, "I/O error");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_011
 * @tc.desc: Verify a stream object without a native entity is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "r");
    EXPECT_CALL(*mock, InstantiateClass(ENV, testing::_, testing::_)).WillOnce(testing::Return(STREAM_OBJECT));
    EXPECT_CALL(*mock, napi_unwrap(ENV, STREAM_OBJECT, testing::_)).WillOnce(testing::Return(napi_generic_failure));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900005, "I/O error");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_012
 * @tc.desc: Verify binary read mode is forwarded unchanged to fopen.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "rb");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
}

/**
 * @tc.name: CreateStreamMockTest_Async_001
 * @tc.desc: Verify Async rejects an argument count outside the two-to-three range.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Async_002
 * @tc.desc: Verify Async rejects a non-path first argument before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Async_003
 * @tc.desc: Verify Async rejects a non-string mode before allocating async work.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(EXISTING_FILE))));
    EXPECT_CALL(*mock, ToUTF8String())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_013
 * @tc.desc: Verify write-update mode creates a readable and writable stream.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(CREATED_FILE, "w+");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
    EXPECT_TRUE(std::filesystem::exists(CREATED_FILE));
}

/**
 * @tc.name: CreateStreamMockTest_Sync_014
 * @tc.desc: Verify append-update mode creates a readable and writable stream.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(CREATED_FILE, "a+");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
    EXPECT_TRUE(std::filesystem::is_regular_file(CREATED_FILE));
}

/**
 * @tc.name: CreateStreamMockTest_Sync_015
 * @tc.desc: Verify binary write mode creates a file and returns its stream.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(CREATED_FILE, "wb");
    ExpectStreamInstantiation();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(CreateStream::Sync(ENV, INFO), STREAM_OBJECT);
    EXPECT_TRUE(std::filesystem::exists(CREATED_FILE));
}

/**
 * @tc.name: CreateStreamMockTest_Sync_016
 * @tc.desc: Verify an empty fopen mode is rejected with invalid argument.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_016, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(EXISTING_FILE, "");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: CreateStreamMockTest_Sync_017
 * @tc.desc: Verify write mode reports a missing parent directory.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamMockTest, CreateStreamMockTest_Sync_017, testing::ext::TestSize.Level1)
{
    constexpr char path[] = "/data/test/CreateStreamMockTest/no-parent/created.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(path, "w");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
