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

#include "move.h"

#include <cerrno>
#include <cstring>
#include <memory>
#include <tuple>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libn_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
namespace {
using OHOS::FileManagement::LibN::NVal;

const napi_env ENV = reinterpret_cast<napi_env>(0x1000);
const napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x2000);
const napi_value UNDEFINED = reinterpret_cast<napi_value>(0x3000);
constexpr char SRC_PATH[] = "/data/test/move_mock_source.txt";
constexpr char DEST_PATH[] = "/data/test/move_mock_destination.txt";

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

class MoveMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
        UvFsMock::EnableMock();
    }

    void TearDown() override
    {
        LibnMock::GetMock()->ResetErrState();
        LibnMock::DisableMock();
        UvFsMock::DisableMock();
    }

    void ExpectTwoValidPaths()
    {
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Return(testing::ByMove(MakeStringResult(SRC_PATH))))
            .WillOnce(testing::Return(testing::ByMove(MakeStringResult(DEST_PATH))));
    }

    void ExpectSuccessfulRename(int mode = MODE_FORCE_MOVE)
    {
        auto mock = LibnMock::GetMock();
        auto uvMock = UvFsMock::GetMock();
        EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
        ExpectTwoValidPaths();
        EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(
            mode == MODE_FORCE_MOVE ? NARG_CNT::TWO : NARG_CNT::THREE));
        if (mode != MODE_FORCE_MOVE) {
            EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, mode)));
            EXPECT_CALL(*uvMock, uv_fs_access(nullptr, testing::_, testing::StrEq(DEST_PATH), 0, nullptr))
                .WillOnce(testing::Return(-ENOENT));
        }
        EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
        EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
            testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(0));
        EXPECT_CALL(*mock, CreateUndefined(ENV)).WillOnce(testing::Return(NVal(ENV, UNDEFINED)));
    }
};

/**
 * @tc.name: MoveMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an argument count outside the public contract.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_002
 * @tc.desc: Verify Sync rejects a source value that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_003
 * @tc.desc: Verify Sync rejects a destination that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(SRC_PATH))))
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a mode whose NAPI conversion fails.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_005
 * @tc.desc: Verify Sync rejects a mode below the supported range.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_006
 * @tc.desc: Verify Sync rejects a mode above the supported range.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, 2)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_007
 * @tc.desc: Verify two-argument Sync performs a force-mode rename.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    ExpectSuccessfulRename();

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(Move::Sync(ENV, INFO), UNDEFINED);
}

/**
 * @tc.name: MoveMockTest_Sync_008
 * @tc.desc: Verify explicit force mode bypasses destination access checking.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, MODE_FORCE_MOVE)));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mock, CreateUndefined(ENV)).WillOnce(testing::Return(NVal(ENV, UNDEFINED)));

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(Move::Sync(ENV, INFO), UNDEFINED);
}

/**
 * @tc.name: MoveMockTest_Sync_009
 * @tc.desc: Verify throw-error mode renames when the destination is absent.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    ExpectSuccessfulRename(MODE_THROW_ERR);

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(Move::Sync(ENV, INFO), UNDEFINED);
}

/**
 * @tc.name: MoveMockTest_Sync_010
 * @tc.desc: Verify throw-error mode refuses to replace an existing destination.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, MODE_THROW_ERR)));
    EXPECT_CALL(*uvMock, uv_fs_access(nullptr, testing::_, testing::StrEq(DEST_PATH), 0, nullptr))
        .WillOnce(testing::Return(0));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900015, "File exists");
}

/**
 * @tc.name: MoveMockTest_Sync_011
 * @tc.desc: Verify throw-error mode propagates an unexpected access failure.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, MODE_THROW_ERR)));
    EXPECT_CALL(*uvMock, uv_fs_access(nullptr, testing::_, testing::StrEq(DEST_PATH), 0, nullptr))
        .WillOnce(testing::Return(-EACCES));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900012, "Permission denied");
}

/**
 * @tc.name: MoveMockTest_Sync_012
 * @tc.desc: Verify a rename permission failure is returned to the caller.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(-EACCES));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900012, "Permission denied");
}

/**
 * @tc.name: MoveMockTest_Sync_013
 * @tc.desc: Verify a missing source reported by rename reaches the NAPI boundary.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(-ENOENT));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: MoveMockTest_Sync_014
 * @tc.desc: Verify a read-only filesystem rename failure is preserved.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(-EROFS));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900027, "Read-only file system");
}

/**
 * @tc.name: MoveMockTest_Sync_015
 * @tc.desc: Verify an invalid rename target error is exposed without producing undefined.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(-EINVAL));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Sync_016
 * @tc.desc: Verify source and destination strings are forwarded unchanged to libuv.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_016, testing::ext::TestSize.Level1)
{
    constexpr char customSrc[] = "/data/test/a file with spaces.txt";
    constexpr char customDest[] = "/data/test/unicode-neutral-destination.txt";
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(customSrc))))
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(customDest))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(customSrc),
        testing::StrEq(customDest), nullptr)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mock, CreateUndefined(ENV)).WillOnce(testing::Return(NVal(ENV, UNDEFINED)));

    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(ENV, testing::_)).Times(0);
    EXPECT_EQ(Move::Sync(ENV, INFO), UNDEFINED);
}

/**
 * @tc.name: MoveMockTest_Sync_017
 * @tc.desc: Verify throw-error mode treats only ENOENT as an absent destination.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_017, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, MODE_THROW_ERR)));
    EXPECT_CALL(*uvMock, uv_fs_access(nullptr, testing::_, testing::StrEq(DEST_PATH), 0, nullptr))
        .WillOnce(testing::Return(-ENOTDIR));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900018, "Not a directory");
}

/**
 * @tc.name: MoveMockTest_Sync_018
 * @tc.desc: Verify force mode preserves a busy-resource error from rename.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Sync_018, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, MODE_FORCE_MOVE)));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(nullptr, testing::_, testing::StrEq(SRC_PATH),
        testing::StrEq(DEST_PATH), nullptr)).WillOnce(testing::Return(-EBUSY));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900014, "Device or resource busy");
}

/**
 * @tc.name: MoveMockTest_Async_001
 * @tc.desc: Verify Async rejects an argument count outside the two-to-four range.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Async_002
 * @tc.desc: Verify Async rejects an invalid source before creating async work.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Async_003
 * @tc.desc: Verify Async rejects an invalid destination before creating async work.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult(SRC_PATH))))
        .WillOnce(testing::Return(testing::ByMove(MakeStringResult("", false))));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Async_004
 * @tc.desc: Verify Async rejects a failed mode conversion before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Async_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: MoveMockTest_Async_005
 * @tc.desc: Verify Async rejects an unsupported move mode before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(MoveMockTest, MoveMockTest_Async_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    ExpectTwoValidPaths();
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::FOUR));
    EXPECT_CALL(*mock, ToInt32(MODE_FORCE_MOVE)).WillOnce(testing::Return(std::make_tuple(true, 99)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);

    EXPECT_EQ(Move::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
