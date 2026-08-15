/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "create_randomaccessfile.h"

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
namespace fs = std::filesystem;

class CreateRandomAccessFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        prctl(PR_SET_NAME, "CreateRafMock");
        LibnMock::EnableMock();
    }

    static void TearDownTestSuite()
    {
        LibnMock::DisableMock();
    }

    void SetUp() override
    {
        fs::create_directories(TEST_ROOT);
    }

    void TearDown() override
    {
        auto mock = LibnMock::GetMock();
        testing::Mock::VerifyAndClearExpectations(mock.get());
        mock->ResetErrState();
        std::error_code error;
        fs::remove_all(TEST_ROOT, error);
    }

    static tuple<bool, unique_ptr<char[]>, size_t> PathResult(const string &path)
    {
        auto value = make_unique<char[]>(path.size() + 1);
        std::copy(path.begin(), path.end(), value.get());
        value[path.size()] = '\0';
        return { true, move(value), path.size() };
    }

    static tuple<bool, unique_ptr<char[]>, size_t> InvalidPathResult()
    {
        return { false, nullptr, 0 };
    }

    static void ExpectPathArgument(const shared_ptr<LibnMock> &mock, const string &path)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillRepeatedly(testing::Return(ARG));
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Return(testing::ByMove(PathResult(path))));
    }

    static void ExpectInvalidFileArgument(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillRepeatedly(testing::Return(ARG));
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Invoke([]() { return InvalidPathResult(); }));
        EXPECT_CALL(*mock, napi_unwrap(ENV, ARG, testing::_))
            .WillOnce(testing::Return(napi_invalid_arg));
    }

    static void CreateFile(const fs::path &path)
    {
        ofstream output(path, ios::binary | ios::trunc);
        ASSERT_TRUE(output.is_open());
        output << "random-access-file";
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x2000);
    static inline napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x2100);
    static inline napi_value ARG = reinterpret_cast<napi_value>(0x2200);
    static inline const fs::path TEST_ROOT = "/data/test/CreateRandomAccessFileMockTest";
};

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_001
 * @tc.desc: Verify the synchronous entry rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_001
 * @tc.desc: Verify the asynchronous entry rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_002
 * @tc.desc: Verify Sync rejects a value that is neither a path nor a File object.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectInvalidFileArgument(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_002
 * @tc.desc: Verify Async rejects a value that is neither a path nor a File object.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectInvalidFileArgument(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_003
 * @tc.desc: Verify Sync rejects a nonnumeric open flag.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "flags.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_003
 * @tc.desc: Verify Async rejects a nonnumeric open flag before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_003, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "flags.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a negative open flag.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "negative_flags.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_004
 * @tc.desc: Verify Async rejects a negative open flag before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_004, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "negative_flags.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_005
 * @tc.desc: Verify Sync rejects a negative options.start value.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "negative_start.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, O_RDONLY)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("start")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("start")).WillOnce(testing::Return(NVal(ENV, ARG)));
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::HasSubstr("option.start"))).Times(1);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_005
 * @tc.desc: Verify Async rejects a negative options.start value.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_005, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "negative_start.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, O_RDONLY)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("start")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("start")).WillOnce(testing::Return(NVal(ENV, ARG)));
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::HasSubstr("option.start"))).Times(1);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_006
 * @tc.desc: Verify Sync rejects a nonnumeric options.end value.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "invalid_end.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, O_RDONLY)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("start")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("end")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("end")).WillOnce(testing::Return(NVal(ENV, ARG)));
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::HasSubstr("option.end"))).Times(1);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Async_006
 * @tc.desc: Verify Async rejects a negative options.end value.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Async_006, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "negative_end.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG));
    EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, O_RDONLY)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("start")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("end")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("end")).WillOnce(testing::Return(NVal(ENV, ARG)));
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::HasSubstr("option.end"))).Times(1);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_007
 * @tc.desc: Verify Sync reports an operating-system error for a missing read-only path.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "missing.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::ONE));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002);
}

/**
 * @tc.name: CreateRandomAccessFileMockTest_Sync_008
 * @tc.desc: Verify Sync reports an instantiation error after opening a valid path.
 * @tc.type: FUNC
 */
HWTEST_F(CreateRandomAccessFileMockTest, CreateRandomAccessFileMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    const fs::path path = TEST_ROOT / "valid.txt";
    CreateFile(path);
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPathArgument(mock, path.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::ONE));
    EXPECT_CALL(*mock, InstantiateClass(ENV, testing::_, testing::_)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(CreateRandomAccessFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900005);
}
} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
