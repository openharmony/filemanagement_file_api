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

#include "listfile_ext_napi.h"

#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;
namespace fs = std::filesystem;

class ListFileExtNapiMockTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        prctl(PR_SET_NAME, "ListFileExtMock");
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

    static void ExpectObjectOptions(const shared_ptr<LibnMock> &mock)
    {
        EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
        EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
        EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(true));
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x4000);
    static inline napi_callback_info INFO = reinterpret_cast<napi_callback_info>(0x4100);
    static inline napi_value ARG_PATH = reinterpret_cast<napi_value>(0x4200);
    static inline napi_value ARG_OPTIONS = reinterpret_cast<napi_value>(0x4300);
    static inline napi_value ARG_FILTER = reinterpret_cast<napi_value>(0x4400);
    static inline const fs::path TEST_ROOT = "/data/test/ListFileExtNapiMockTest";
};

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_001
 * @tc.desc: Verify Sync rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_001
 * @tc.desc: Verify Async rejects an unmatched argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_002
 * @tc.desc: Verify Sync rejects a value that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_PATH));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Invoke([]() { return InvalidPathResult(); }));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_002
 * @tc.desc: Verify Async rejects a value that cannot be converted to a path.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_002, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(ARG_PATH));
    EXPECT_CALL(*mock, ToUTF8StringPath())
        .WillOnce(testing::Invoke([]() { return InvalidPathResult(); }));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_003
 * @tc.desc: Verify Sync rejects an options value of an unsupported type.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_003
 * @tc.desc: Verify Async rejects an options value of an unsupported type.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_003, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_004
 * @tc.desc: Verify Sync rejects a nonnumeric listNum option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_004
 * @tc.desc: Verify Async rejects a nonnumeric listNum option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_004, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(false, 0)));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_005
 * @tc.desc: Verify Sync rejects a nonboolean recursion option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_005, testing::ext::TestSize.Level1)
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
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_005
 * @tc.desc: Verify Async rejects a nonboolean recursion option.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_005, testing::ext::TestSize.Level1)
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
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_006
 * @tc.desc: Verify Sync rejects a fileFilter without a callable filter member.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileFilter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileFilter")).WillOnce(testing::Return(NVal(ENV, ARG_FILTER)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_FILTER)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Async_006
 * @tc.desc: Verify Async rejects a fileFilter without a callable filter member.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Async_006, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, TEST_ROOT.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileFilter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileFilter")).WillOnce(testing::Return(NVal(ENV, ARG_FILTER)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, GetProp("filter")).WillOnce(testing::Return(NVal(ENV, ARG_FILTER)));
    EXPECT_CALL(*mock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Async(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_007
 * @tc.desc: Verify undefined options are accepted before a missing-directory error.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_007, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "undefined_options";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    EXPECT_CALL(*mock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(ARG_OPTIONS));
    EXPECT_CALL(*mock, TypeIs(napi_object)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_008
 * @tc.desc: Verify an undefined fileFilter is ignored before traversal.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_008, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "undefined_filter";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, HasProp("fileFilter")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("fileFilter")).WillOnce(testing::Return(NVal(ENV, ARG_FILTER)));
    EXPECT_CALL(*mock, TypeIs(napi_undefined)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}

/**
 * @tc.name: ListFileExtNapiMockTest_Sync_009
 * @tc.desc: Verify valid scalar options reach the core traversal layer.
 * @tc.type: FUNC
 */
HWTEST_F(ListFileExtNapiMockTest, ListFileExtNapiMockTest_Sync_009, testing::ext::TestSize.Level1)
{
    const fs::path missing = TEST_ROOT / "valid_scalar_options";
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(testing::Return(true));
    ExpectPath(mock, missing.string());
    ExpectObjectOptions(mock);
    EXPECT_CALL(*mock, HasProp("listNum")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("listNum")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToInt64(0)).WillOnce(testing::Return(make_tuple(true, 5)));
    EXPECT_CALL(*mock, HasProp("recursion")).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetProp("recursion")).WillOnce(testing::Return(NVal(ENV, ARG_OPTIONS)));
    EXPECT_CALL(*mock, ToBool(false)).WillOnce(testing::Return(make_tuple(true, true)));
    EXPECT_CALL(*mock, HasProp("fileFilter")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(ENV)).Times(1);
    EXPECT_EQ(ListFileExtNapi::Sync(ENV, INFO), nullptr);
    mock->VerifyAndClearErr(13900002, "No such file or directory");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
