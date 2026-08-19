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

#include "listfile.h"

#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;
namespace fs = std::filesystem;

class ListFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        prctl(PR_SET_NAME, "ListFileMockTest");
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

    static void ExpectPath(const shared_ptr<LibnMock> &mock, const string &path)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_PATH));
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Return(testing::ByMove(PathResult(path))));
    }

    static void ExpectInvalidPath(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_PATH));
        EXPECT_CALL(*mock, ToUTF8StringPath())
            .WillOnce(testing::Invoke([]() { return InvalidPathResult(); }));
    }

    static void ExpectObjectOptions(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
        EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
        EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(true));
    }

    static void ExpectNoFilterProperties(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(false));
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x3000);
    static inline napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x3100);
    static inline napi_value ARG_PATH = reinterpret_cast<napi_value>(0x3200);
    static inline napi_value ARG_OPTIONS = reinterpret_cast<napi_value>(0x3300);
    static inline const fs::path TEST_ROOT = "/data/test/ListFileMockTest";
};

/**
 * @tc.name: ListFileMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_001
 * @tc.desc: Verify Async rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_002
 * @tc.desc: Verify Sync rejects a value that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectInvalidPath(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_002
 * @tc.desc: Verify Async rejects a value that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectInvalidPath(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_003
 * @tc.desc: Verify Sync rejects an options argument of an unsupported type.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_003
 * @tc.desc: Verify Async rejects an options argument of an unsupported type.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a nonnumeric listNum option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_004
 * @tc.desc: Verify Async rejects a negative listNum option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_005
 * @tc.desc: Verify Sync rejects a nonboolean recursion option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("recursion")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToBool(false)).WillOnce(testing::Return(make_tuple(false, false)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_005
 * @tc.desc: Verify Async rejects a nonboolean recursion option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_005, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("recursion")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToBool(false)).WillOnce(testing::Return(make_tuple(false, false)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_006
 * @tc.desc: Verify Sync accepts undefined options and reports a missing directory.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_007
 * @tc.desc: Verify Sync accepts a callback-shaped second argument and reports a missing directory.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing_callback";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_008
 * @tc.desc: Verify listNum zero reaches directory traversal without being rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing_list_zero";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(true, 0)));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    ExpectNoFilterProperties(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_009
 * @tc.desc: Verify a positive listNum reaches directory traversal.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing_list_positive";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(true, 10)));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    ExpectNoFilterProperties(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_010
 * @tc.desc: Verify recursion false uses nonrecursive traversal.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_010, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing_nonrecursive";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("recursion")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToBool(false)).WillOnce(testing::Return(make_tuple(true, false)));
    ExpectNoFilterProperties(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_011
 * @tc.desc: Verify recursion true uses recursive traversal.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_011, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "missing_recursive";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("recursion")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToBool(false)).WillOnce(testing::Return(make_tuple(true, true)));
    ExpectNoFilterProperties(mock);
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileMockTest_Sync_012
 * @tc.desc: Verify a suffix filter must be convertible to a string array.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray()).WillOnce(testing::Return(make_tuple(false, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_013
 * @tc.desc: Verify an empty suffix filter is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray()).WillOnce(testing::Return(make_tuple(true, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_014
 * @tc.desc: Verify a suffix filter must start with a dot.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { "txt" }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_015
 * @tc.desc: Verify a suffix filter rejects punctuation after its leading dot.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { ".tar.gz" }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_016
 * @tc.desc: Verify an empty displayName filter is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_016, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("displayName")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray()).WillOnce(testing::Return(make_tuple(true, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_017
 * @tc.desc: Verify a negative fileSizeOver filter is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_017, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileSizeOver")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_018
 * @tc.desc: Verify a negative lastModifiedAfter filter is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_018, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("lastModifiedAfter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("lastModifiedAfter"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToDouble()).WillOnce(testing::Return(make_tuple(true, -0.5)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_019
 * @tc.desc: Verify a suffix containing only the leading dot is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_019, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { "." }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_020
 * @tc.desc: Verify a suffix longer than the public limit is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_020, testing::ext::TestSize.Level1)
{
    const string suffix = "." + string(MAX_SUFFIX_LENGTH, 'a');
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { suffix }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_021
 * @tc.desc: Verify displayName must be convertible to a string array.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_021, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("displayName"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(false, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_022
 * @tc.desc: Verify fileSizeOver must be convertible to a signed integer.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_022, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileSizeOver"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Sync_023
 * @tc.desc: Verify lastModifiedAfter must be convertible to a number.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Sync_023, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("lastModifiedAfter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("lastModifiedAfter"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToDouble()).WillOnce(testing::Return(make_tuple(false, 0.0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_006
 * @tc.desc: Verify Async rejects a suffix that is not a string array before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(false, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_007
 * @tc.desc: Verify Async rejects an empty suffix array before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_007, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_008
 * @tc.desc: Verify Async rejects a suffix without a leading dot before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_008, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { "log" }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_009
 * @tc.desc: Verify Async rejects punctuation in a suffix before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_009, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { ".tar.gz" }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_010
 * @tc.desc: Verify Async rejects a non-array displayName before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_010, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("displayName"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(false, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_011
 * @tc.desc: Verify Async rejects an empty displayName array before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_011, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("displayName"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> {}, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_012
 * @tc.desc: Verify Async rejects a nonnumeric fileSizeOver before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_012, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileSizeOver"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_013
 * @tc.desc: Verify Async rejects a negative fileSizeOver before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_013, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileSizeOver"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToInt64()).WillOnce(testing::Return(make_tuple(true, -1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_014
 * @tc.desc: Verify Async rejects a nonnumeric lastModifiedAfter before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_014, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("lastModifiedAfter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("lastModifiedAfter"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToDouble()).WillOnce(testing::Return(make_tuple(false, 0.0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_015
 * @tc.desc: Verify Async rejects a negative lastModifiedAfter before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_015, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("displayName")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileSizeOver")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("lastModifiedAfter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("lastModifiedAfter"))
        .Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToDouble()).WillOnce(testing::Return(make_tuple(true, -0.5)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileMockTest_Async_016
 * @tc.desc: Verify Async rejects a suffix containing only a leading dot before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_016, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("filter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, HasProp("suffix")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("suffix")).Times(2).WillRepeatedly(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined))
        .WillOnce(testing::Return(false)).RetiresOnSaturation();
    EXPECT_CALL(*mock, ToStringArray())
        .WillOnce(testing::Return(make_tuple(true, vector<string> { "." }, 1)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFile::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
