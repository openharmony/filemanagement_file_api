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

#include "copy_file.h"

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
using namespace OHOS::FileManagement::ModuleFileIO;
namespace fs = std::filesystem;

class CopyFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        prctl(PR_SET_NAME, "CopyFileMockTest");
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

    static void WriteFile(const fs::path &path, const string &content)
    {
        ofstream output(path, ios::binary | ios::trunc);
        ASSERT_TRUE(output.is_open());
        output << content;
        output.close();
    }

    static string ReadFile(const fs::path &path)
    {
        ifstream input(path, ios::binary);
        return { istreambuf_iterator<char>(input), istreambuf_iterator<char>() };
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x1000);
    static inline napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x1100);
    static inline napi_value ARG = reinterpret_cast<napi_value>(0x1200);
    static inline const fs::path TEST_ROOT = "/data/test/CopyFileMockTest";
};

/**
 * @tc.name: CopyFileMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Async_001
 * @tc.desc: Verify Async rejects an invalid argument count before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Async(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Sync_002
 * @tc.desc: Verify Sync rejects operands that are neither paths nor file descriptors.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).Times(2).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath()).Times(2)
        .WillRepeatedly(testing::Invoke([]() { return InvalidPathResult(); }));
    EXPECT_CALL(*mock, ToInt32()).Times(2).WillRepeatedly(testing::Return(make_tuple(false, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Async_002
 * @tc.desc: Verify Async rejects operands that are neither paths nor file descriptors.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).Times(2).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath()).Times(2)
        .WillRepeatedly(testing::Invoke([]() { return InvalidPathResult(); }));
    EXPECT_CALL(*mock, ToInt32()).Times(2).WillRepeatedly(testing::Return(make_tuple(false, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Async(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Sync_003
 * @tc.desc: Verify Sync rejects a nonzero copy mode.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(PathResult((TEST_ROOT / "src").string()))))
        .WillOnce(testing::Return(testing::ByMove(PathResult((TEST_ROOT / "dst").string()))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(true, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Async_003
 * @tc.desc: Verify Async rejects an invalid copy mode before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(PathResult((TEST_ROOT / "src").string()))))
        .WillOnce(testing::Return(testing::ByMove(PathResult((TEST_ROOT / "dst").string()))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock, ToInt32(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Async(ENV, INFO), nullptr);
}

/**
 * @tc.name: CopyFileMockTest_Sync_004
 * @tc.desc: Verify Sync copies the complete contents between valid paths.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    const fs::path source = TEST_ROOT / "source.txt";
    const fs::path destination = TEST_ROOT / "destination.txt";
    const string content = "copy-file-content-中文";
    WriteFile(source, content);

    napi_value expected = reinterpret_cast<napi_value>(0x1300);
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(PathResult(source.string()))))
        .WillOnce(testing::Return(testing::ByMove(PathResult(destination.string()))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, CreateUndefined(ENV)).WillOnce(testing::Return(NVal(ENV, expected)));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), expected);
    EXPECT_EQ(ReadFile(destination), content);
}

/**
 * @tc.name: CopyFileMockTest_Sync_005
 * @tc.desc: Verify Sync reports a missing source path.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    const fs::path source = TEST_ROOT / "missing.txt";
    const fs::path destination = TEST_ROOT / "destination.txt";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(PathResult(source.string()))))
        .WillOnce(testing::Return(testing::ByMove(PathResult(destination.string()))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, ThrowErr(ENV));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), nullptr);
    EXPECT_FALSE(fs::exists(destination));
}

/**
 * @tc.name: CopyFileMockTest_Sync_006
 * @tc.desc: Verify Sync replaces an existing destination when mode is zero.
 * @tc.type: FUNC
 */
HWTEST_F(CopyFileMockTest, CopyFileMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    const fs::path source = TEST_ROOT / "source.bin";
    const fs::path destination = TEST_ROOT / "destination.bin";
    WriteFile(source, "new-data");
    WriteFile(destination, "old-data");

    napi_value expected = reinterpret_cast<napi_value>(0x1400);
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(testing::_)).WillRepeatedly(testing::Return(ARG));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Return(testing::ByMove(PathResult(source.string()))))
        .WillOnce(testing::Return(testing::ByMove(PathResult(destination.string()))));
    EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, CreateUndefined(ENV)).WillOnce(testing::Return(NVal(ENV, expected)));

    EXPECT_EQ(CopyFile::Sync(ENV, INFO), expected);
    EXPECT_EQ(ReadFile(destination), "new-data");
}
} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
