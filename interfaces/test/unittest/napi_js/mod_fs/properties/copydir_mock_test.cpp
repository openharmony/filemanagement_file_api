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

#include "copydir.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libn_mock.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;
namespace fs = std::filesystem;

static tuple<bool, unique_ptr<char[]>, size_t> MakePath(const fs::path &path);

class CopyDirMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
        fs::create_directories(TEST_ROOT / "src");
        fs::create_directories(TEST_ROOT / "dest");
    }

    void TearDown() override
    {
        LibnMock::DisableMock();
        std::error_code err;
        fs::remove_all(TEST_ROOT, err);
    }

    static inline const fs::path TEST_ROOT = "/data/test/CopyDirMockTest";

    void ExpectValidOperands(const fs::path &src, const fs::path &dest, size_t argc = NARG_CNT::TWO,
        int mode = DIRMODE_FILE_COPY_THROW_ERR)
    {
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
        EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Return(MakePath(src)))
            .WillOnce(testing::Return(MakePath(dest)));
        EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(argc));
        if (argc >= NARG_CNT::THREE) {
            EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD))
                .WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1300)));
            EXPECT_CALL(*mock, ToInt32(testing::_)).WillOnce(testing::Return(make_tuple(true, mode)));
        }
    }

    void ExpectUndefined(napi_value result)
    {
        EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
            .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), result)));
    }

    static void WriteText(const fs::path &path, const string &text)
    {
        fs::create_directories(path.parent_path());
        ofstream stream(path);
        stream << text;
    }

    static string ReadText(const fs::path &path)
    {
        ifstream stream(path);
        return string(istreambuf_iterator<char>(stream), istreambuf_iterator<char>());
    }
};

static tuple<bool, unique_ptr<char[]>, size_t> MakePath(const fs::path &path)
{
    string value = path.string();
    auto result = make_unique<char[]>(value.size() + 1);
    if (strcpy_s(result.get(), value.size() + 1, value.c_str()) != EOK) {
        return { false, nullptr, 0 };
    }
    return { true, move(result), value.size() };
}

/**
 * @tc.name: CopyDirMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_002
 * @tc.desc: Verify Sync rejects an invalid source path.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(make_tuple(false, unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_003
 * @tc.desc: Verify Sync rejects identical source and destination paths.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a destination nested under the source.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    fs::create_directories(TEST_ROOT / "src/child");
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src/child")));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_005
 * @tc.desc: Verify Sync rejects an out-of-range copy mode.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "dest")));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(testing::_))
        .WillOnce(testing::Return(make_tuple(true, COPYMODE_MAX + 1)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Async_001
 * @tc.desc: Verify Async rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_006
 * @tc.desc: Verify Sync rejects a missing source directory.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, ToUTF8StringPath()).WillOnce(testing::Return(MakePath(TEST_ROOT / "missing")));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_007
 * @tc.desc: Verify Sync rejects a regular file used as the source directory.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    WriteText(TEST_ROOT / "source-file.txt", "source");
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, ToUTF8StringPath()).WillOnce(testing::Return(MakePath(TEST_ROOT / "source-file.txt")));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_008
 * @tc.desc: Verify Sync rejects an invalid destination path conversion.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(make_tuple(false, unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_009
 * @tc.desc: Verify Sync rejects a missing destination directory.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "missing-dest")));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_010
 * @tc.desc: Verify Sync rejects copying a directory into its parent.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_011
 * @tc.desc: Verify Sync rejects a negative copy mode.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest", NARG_CNT::THREE, COPYMODE_MIN - 1);
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_012
 * @tc.desc: Verify Sync rejects a copy mode that cannot convert to int32.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::THIRD)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1300)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "dest")));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(testing::_)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_013
 * @tc.desc: Verify Sync copies an empty directory.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::is_directory(TEST_ROOT / "dest/src"));
}

/**
 * @tc.name: CopyDirMockTest_Sync_014
 * @tc.desc: Verify Sync copies a file and preserves its contents.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    WriteText(TEST_ROOT / "src/file.txt", "copy-dir-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src/file.txt"), "copy-dir-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_015
 * @tc.desc: Verify Sync recursively copies nested directories and files.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    WriteText(TEST_ROOT / "src/level1/level2/nested.txt", "nested-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src/level1/level2/nested.txt"), "nested-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_016
 * @tc.desc: Verify replace mode overwrites an existing destination file.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_016, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    WriteText(TEST_ROOT / "src/file.txt", "new-content");
    WriteText(TEST_ROOT / "dest/src/file.txt", "old-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest", NARG_CNT::THREE, DIRMODE_FILE_COPY_REPLACE);
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src/file.txt"), "new-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_017
 * @tc.desc: Verify replace mode preserves unrelated destination files.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_017, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    WriteText(TEST_ROOT / "src/source.txt", "source");
    WriteText(TEST_ROOT / "dest/src/unrelated.txt", "unrelated");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest", NARG_CNT::THREE, DIRMODE_FILE_COPY_REPLACE);
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src/source.txt"), "source");
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src/unrelated.txt"), "unrelated");
}

/**
 * @tc.name: CopyDirMockTest_Async_002
 * @tc.desc: Verify Async rejects an invalid source path before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(make_tuple(false, unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Async_003
 * @tc.desc: Verify Async rejects an invalid destination path before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1200)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(MakePath(TEST_ROOT / "src")))
        .WillOnce(testing::Return(make_tuple(false, unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Async_004
 * @tc.desc: Verify Async rejects an invalid explicit mode before scheduling work.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Async_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest", NARG_CNT::THREE, COPYMODE_MAX + 1);
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(CopyDir::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020);
}

/**
 * @tc.name: CopyDirMockTest_Sync_018
 * @tc.desc: Verify Sync copies a hidden file.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_018, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = ".hidden";
    WriteText(TEST_ROOT / "src" / relativePath, "hidden-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "hidden-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_019
 * @tc.desc: Verify Sync copies a file whose name contains spaces.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_019, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "file with spaces.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "space-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "space-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_020
 * @tc.desc: Verify Sync copies a non-ASCII file name.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_020, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "中文文件.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "unicode-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "unicode-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_021
 * @tc.desc: Verify Sync copies a zero-length file.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_021, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "empty.bin";
    WriteText(TEST_ROOT / "src" / relativePath, "");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "");
}

/**
 * @tc.name: CopyDirMockTest_Sync_022
 * @tc.desc: Verify Sync copies a file in a hidden directory.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_022, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = ".config/settings.json";
    WriteText(TEST_ROOT / "src" / relativePath, "{enabled:true}");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "{enabled:true}");
}

/**
 * @tc.name: CopyDirMockTest_Sync_023
 * @tc.desc: Verify Sync copies a deeply nested file.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_023, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "a/b/c/d/e/deep.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "deep-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "deep-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_024
 * @tc.desc: Verify Sync copies punctuation in a file name.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_024, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "name-_.+@.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "punctuation-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "punctuation-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_025
 * @tc.desc: Verify Sync copies files under two sibling levels.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_025, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "left/branch/file.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "left-branch");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "left-branch");
}

/**
 * @tc.name: CopyDirMockTest_Sync_026
 * @tc.desc: Verify Sync copies an extensionless file.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_026, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "README";
    WriteText(TEST_ROOT / "src" / relativePath, "readme-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "readme-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_027
 * @tc.desc: Verify Sync copies a UTF-8 directory name.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_027, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "数据/记录.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "directory-unicode");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "directory-unicode");
}

/**
 * @tc.name: CopyDirMockTest_Sync_028
 * @tc.desc: Verify Sync copies a file with a numeric name.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_028, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "20260724/0001";
    WriteText(TEST_ROOT / "src" / relativePath, "numeric-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "numeric-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_029
 * @tc.desc: Verify Sync copies a dot in a directory name.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_029, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "config.d/item.conf";
    WriteText(TEST_ROOT / "src" / relativePath, "config-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "config-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_030
 * @tc.desc: Verify Sync copies a mixed-case path.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_030, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "MixedCase/File.TxT";
    WriteText(TEST_ROOT / "src" / relativePath, "mixed-case-content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "mixed-case-content");
}

/**
 * @tc.name: CopyDirMockTest_Sync_031
 * @tc.desc: Verify Sync copies a file below an existing empty branch.
 * @tc.type: FUNC
 */
HWTEST_F(CopyDirMockTest, CopyDirMockTest_Sync_031, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    const fs::path relativePath = "existing/child.txt";
    WriteText(TEST_ROOT / "src" / relativePath, "existing-branch");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectValidOperands(TEST_ROOT / "src", TEST_ROOT / "dest");
    ExpectUndefined(expected);

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(CopyDir::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_TRUE(fs::exists(TEST_ROOT / "dest/src" / relativePath));
    EXPECT_EQ(ReadText(TEST_ROOT / "dest/src" / relativePath), "existing-branch");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
