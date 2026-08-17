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

#include "rmdirent.h"

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

static tuple<bool, unique_ptr<char[]>, size_t> MakeRmdirPath(const fs::path &path)
{
    string value = path.string();
    auto result = make_unique<char[]>(value.size() + 1);
    if (strcpy_s(result.get(), value.size() + 1, value.c_str()) != EOK) {
        return { false, nullptr, 0 };
    }
    return { true, move(result), value.size() };
}

class RmdirentMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
        fs::create_directories(TEST_ROOT);
    }

    void TearDown() override
    {
        LibnMock::DisableMock();
        error_code error;
        fs::remove_all(TEST_ROOT, error);
    }

    void ExpectSyncPath(const fs::path &path)
    {
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST))
            .WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
        EXPECT_CALL(*mock, ToUTF8StringPath()).WillOnce(testing::Return(MakeRmdirPath(path)));
    }

    static void WriteText(const fs::path &path, const string &text)
    {
        fs::create_directories(path.parent_path());
        ofstream stream(path);
        stream << text;
    }

    static inline const fs::path TEST_ROOT = "/data/test/RmdirentMockTest";
};

/**
 * @tc.name: RmdirentMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: RmdirentMockTest_Sync_002
 * @tc.desc: Verify Sync rejects an invalid path.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(std::make_tuple(false, std::unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: RmdirentMockTest_Async_001
 * @tc.desc: Verify Async rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: RmdirentMockTest_Async_002
 * @tc.desc: Verify Async rejects an invalid path.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(std::make_tuple(false, std::unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: RmdirentMockTest_Sync_003
 * @tc.desc: Verify Sync removes an empty directory.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "empty";
    fs::create_directories(target);
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_004
 * @tc.desc: Verify Sync recursively removes nested empty directories.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "nested";
    fs::create_directories(target / "one/two/three");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_005
 * @tc.desc: Verify Sync recursively removes a directory containing one file.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "one-file";
    WriteText(target / "file.txt", "content");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_006
 * @tc.desc: Verify Sync recursively removes files at multiple nesting levels.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "tree";
    WriteText(target / "root.txt", "root");
    WriteText(target / "child/child.txt", "child");
    WriteText(target / "child/grandchild/leaf.txt", "leaf");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_007
 * @tc.desc: Verify Sync reports an error when the target does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "does-not-exist";
    ExpectSyncPath(target);
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_008
 * @tc.desc: Verify Sync accepts a trailing slash for an existing directory.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "trailing-slash";
    fs::create_directories(target);
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target.string() + "/");
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_009
 * @tc.desc: Verify Sync removes directories containing hidden files.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "hidden";
    WriteText(target / ".hidden-file", "hidden");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_010
 * @tc.desc: Verify Sync removes directories containing non-ASCII file names.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "unicode";
    WriteText(target / "测试文件.txt", "unicode");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Async_003
 * @tc.desc: Verify Async rejects a failed path conversion before scheduling a promise.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST))
        .WillOnce(testing::Return(reinterpret_cast<napi_value>(0x1100)));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(make_tuple(false, unique_ptr<char[]>(), 0)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(Rmdirent::Async(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: RmdirentMockTest_Sync_011
 * @tc.desc: Verify Sync removes a directory whose name contains spaces.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "directory with spaces";
    WriteText(target / "file.txt", "payload-011");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_012
 * @tc.desc: Verify Sync removes a non-ASCII directory.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "中文目录";
    WriteText(target / "文件.txt", "payload-012");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_013
 * @tc.desc: Verify Sync recursively removes a directory containing a non-empty file.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "empty-file";
    WriteText(target / "empty.bin", "payload-013");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_014
 * @tc.desc: Verify Sync removes a hidden directory tree.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / ".hidden-tree";
    WriteText(target / ".hidden-file", "payload-014");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_015
 * @tc.desc: Verify Sync removes a mixed-case directory tree.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "MixedCase";
    WriteText(target / "File.TxT", "payload-015");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_016
 * @tc.desc: Verify Sync removes a punctuation-heavy tree.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_016, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "name-_.+@";
    WriteText(target / "item.conf", "payload-016");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_017
 * @tc.desc: Verify Sync removes a numeric directory tree.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_017, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "20260724/0001";
    WriteText(target / "data", "payload-017");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_018
 * @tc.desc: Verify Sync removes a configuration directory.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_018, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "config.d/sub";
    WriteText(target / "item.conf", "payload-018");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_019
 * @tc.desc: Verify Sync removes a deep directory tree.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_019, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "a/b/c/d/e/f";
    WriteText(target / "leaf.txt", "payload-019");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}

/**
 * @tc.name: RmdirentMockTest_Sync_020
 * @tc.desc: Verify Sync removes a directory containing a README.
 * @tc.type: FUNC
 */
HWTEST_F(RmdirentMockTest, RmdirentMockTest_Sync_020, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    fs::path target = TEST_ROOT / "project/docs";
    WriteText(target / "README", "payload-020");
    napi_value expected = reinterpret_cast<napi_value>(0x2000);
    ExpectSyncPath(target);
    EXPECT_CALL(*LibnMock::GetMock(), CreateUndefined(testing::_))
        .WillOnce(testing::Return(LibN::NVal(reinterpret_cast<napi_env>(0x1000), expected)));

    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*mock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);
    EXPECT_EQ(Rmdirent::Sync(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
    EXPECT_FALSE(fs::exists(target));
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
