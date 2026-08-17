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

#include "fdopen_stream.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "libn_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;

class FdopenStreamMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        prctl(PR_SET_NAME, "FdopenStreamMock");
        LibnMock::EnableMock();
    }

    static void TearDownTestSuite()
    {
        LibnMock::DisableMock();
    }

    void TearDown() override
    {
        auto mock = LibnMock::GetMock();
        testing::Mock::VerifyAndClearExpectations(mock.get());
        mock->ResetErrState();
    }

    static tuple<bool, unique_ptr<char[]>, size_t> ModeResult(const string &mode)
    {
        auto value = make_unique<char[]>(mode.size() + 1);
        std::copy(mode.begin(), mode.end(), value.get());
        value[mode.size()] = '\0';
        return { true, move(value), mode.size() };
    }

    static tuple<bool, unique_ptr<char[]>, size_t> InvalidModeResult()
    {
        return { false, nullptr, 0 };
    }

    static void ExpectArguments(const shared_ptr<LibnMock> &mock, int fd, const string &mode)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_FD));
        EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_MODE));
        EXPECT_CALL(*mock, ToInt32()).WillOnce(testing::Return(make_tuple(true, fd)));
        EXPECT_CALL(*mock, ToUTF8String())
            .WillOnce(testing::Return(testing::ByMove(ModeResult(mode))));
    }

    static void ExpectInvalidFd(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_FD));
        EXPECT_CALL(*mock, ToInt32()).WillOnce(testing::Return(make_tuple(false, -1)));
    }

    static void ExpectInvalidMode(const shared_ptr<LibnMock> &mock, int fd)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_FD));
        EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_MODE));
        EXPECT_CALL(*mock, ToInt32()).WillOnce(testing::Return(make_tuple(true, fd)));
        EXPECT_CALL(*mock, ToUTF8String())
            .WillOnce(testing::Invoke([]() { return InvalidModeResult(); }));
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x5000);
    static inline napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x5100);
    static inline napi_value ARG_FD = reinterpret_cast<napi_value>(0x5200);
    static inline napi_value ARG_MODE = reinterpret_cast<napi_value>(0x5300);
};

/**
 * @tc.name: FdopenStreamMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Async_001
 * @tc.desc: Verify Async rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_002
 * @tc.desc: Verify Sync rejects an argument that cannot be converted to an fd.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectInvalidFd(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Async_002
 * @tc.desc: Verify Async rejects an argument that cannot be converted to an fd.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectInvalidFd(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_003
 * @tc.desc: Verify Sync rejects a negative fd even when conversion succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(mock, -1, "r");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Async_003
 * @tc.desc: Verify Async rejects a negative fd before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectArguments(mock, -1, "r");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a mode that cannot be converted to text.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectInvalidMode(mock, STDIN_FILENO);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Async_004
 * @tc.desc: Verify Async rejects a mode that cannot be converted to text.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Async_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectInvalidMode(mock, STDIN_FILENO);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_005
 * @tc.desc: Verify Sync propagates EBADF when fdopen receives a closed descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    int fd = dup(STDIN_FILENO);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);

    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(mock, fd, "r");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900008, "Bad file descriptor");
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_006
 * @tc.desc: Verify Sync propagates EINVAL for an unsupported mode string.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    int fd = dup(STDIN_FILENO);
    ASSERT_GE(fd, 0);

    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(mock, fd, "invalid-mode");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(close(fd), 0);
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_007
 * @tc.desc: Verify an empty mode is rejected by the C stream boundary.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    int fd = dup(STDIN_FILENO);
    ASSERT_GE(fd, 0);

    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(mock, fd, "");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(close(fd), 0);
}

/**
 * @tc.name: FdopenStreamMockTest_Sync_008
 * @tc.desc: Verify a malformed update mode is rejected by the C stream boundary.
 * @tc.type: FUNC
 */
HWTEST_F(FdopenStreamMockTest, FdopenStreamMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    int fd = dup(STDIN_FILENO);
    ASSERT_GE(fd, 0);

    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectArguments(mock, fd, "q");
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(FdopenStream::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(close(fd), 0);
}
} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
