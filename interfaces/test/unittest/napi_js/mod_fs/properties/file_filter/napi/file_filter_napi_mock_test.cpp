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

#include "file_filter_napi.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
namespace {
constexpr size_t SCOPE_RESULT_ARG_INDEX = 1;
constexpr size_t VALUE_RESULT_ARG_INDEX = 2;
constexpr size_t REFERENCE_RESULT_ARG_INDEX = 3;
constexpr size_t FUNCTION_RESULT_ARG_INDEX = 5;
} // namespace

class FileFilterNapiMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
        mock_ = LibnMock::GetMock();
    }

    void TearDown() override
    {
        LibnMock::DisableMock();
    }

    void ExpectConstructor(napi_status status = napi_ok)
    {
        EXPECT_CALL(*mock_, napi_create_reference(ENV, CALLBACK_INPUT, 1, testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<REFERENCE_RESULT_ARG_INDEX>(CALLBACK_REFERENCE), testing::Return(napi_ok)));
        EXPECT_CALL(*mock_, napi_create_reference(ENV, FILTER_OBJECT, 1, testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<REFERENCE_RESULT_ARG_INDEX>(FILTER_REFERENCE), testing::Return(status)));
    }

    void ExpectDestructor(bool hasFilterReference = true)
    {
        if (hasFilterReference) {
            EXPECT_CALL(*mock_, napi_delete_reference(ENV, FILTER_REFERENCE)).WillOnce(testing::Return(napi_ok));
        }
        EXPECT_CALL(*mock_, napi_delete_reference(ENV, CALLBACK_REFERENCE)).WillOnce(testing::Return(napi_ok));
    }

    void ExpectOpenScope(napi_status status = napi_ok)
    {
        EXPECT_CALL(*mock_, napi_open_handle_scope(ENV, testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<SCOPE_RESULT_ARG_INDEX>(HANDLE_SCOPE), testing::Return(status)));
    }

    void ExpectCallbackPrelude()
    {
        EXPECT_CALL(*mock_, CreateUTF8String(ENV, testing::_))
            .WillOnce(testing::Return(LibN::NVal(ENV, NAME_VALUE)));
        EXPECT_CALL(*mock_, napi_get_reference_value(ENV, CALLBACK_REFERENCE, testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(CALLBACK_VALUE), testing::Return(napi_ok)));
    }

    void ExpectCloseScope()
    {
        EXPECT_CALL(*mock_, napi_close_handle_scope(ENV, HANDLE_SCOPE)).WillOnce(testing::Return(napi_ok));
    }

    static inline napi_env ENV = reinterpret_cast<napi_env>(0x1000);
    static inline napi_value FILTER_OBJECT = reinterpret_cast<napi_value>(0x1100);
    static inline napi_ref FILTER_REFERENCE = reinterpret_cast<napi_ref>(0x1200);
    static inline napi_value CALLBACK_INPUT = reinterpret_cast<napi_value>(0x1250);
    static inline napi_ref CALLBACK_REFERENCE = reinterpret_cast<napi_ref>(0x1300);
    static inline napi_value CALLBACK_VALUE = reinterpret_cast<napi_value>(0x1400);
    static inline napi_value NAME_VALUE = reinterpret_cast<napi_value>(0x1500);
    static inline napi_value FILTER_RESULT = reinterpret_cast<napi_value>(0x1600);
    static inline napi_handle_scope HANDLE_SCOPE = reinterpret_cast<napi_handle_scope>(0x1700);

    std::shared_ptr<LibnMock> mock_;
};

/**
 * @tc.name: FileFilterNapiMockTest_HasException_001
 * @tc.desc: Verify a newly constructed filter has no captured exception.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_HasException_001, testing::ext::TestSize.Level1)
{
    ExpectConstructor(napi_generic_failure);
    ExpectDestructor(false);
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.HasException());
}

/**
 * @tc.name: FileFilterNapiMockTest_HandleException_001
 * @tc.desc: Verify HandleException returns null when no exception was captured.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_HandleException_001, testing::ext::TestSize.Level1)
{
    ExpectConstructor(napi_generic_failure);
    ExpectDestructor(false);
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_EQ(filter.HandleException(ENV), nullptr);
}

/**
 * @tc.name: FileFilterNapiMockTest_Constructor_001
 * @tc.desc: Verify a successful constructor owns and releases the filter reference.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Constructor_001, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    LibN::NVal callback(ENV, CALLBACK_INPUT);

    {
        FileFilterNapi filter(ENV, FILTER_OBJECT, callback);
        EXPECT_FALSE(filter.HasException());
    }
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_001
 * @tc.desc: Verify Filter fails immediately when opening a handle scope fails.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_001, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope(napi_generic_failure);
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("alpha.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_002
 * @tc.desc: Verify Filter remembers an open-scope failure and avoids a second NAPI call.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_002, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope(napi_generic_failure);
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("first.txt"));
    EXPECT_FALSE(filter.Filter("second.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_003
 * @tc.desc: Verify Filter closes its scope when the filter object cannot be dereferenced.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_003, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("missing.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_004
 * @tc.desc: Verify Filter closes its scope when the JavaScript callback fails.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_004, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(FILTER_OBJECT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_call_function(ENV, FILTER_OBJECT, CALLBACK_VALUE, 1, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("callback-error.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_005
 * @tc.desc: Verify Filter rejects a callback result that cannot convert to boolean.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_005, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(FILTER_OBJECT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_call_function(ENV, FILTER_OBJECT, CALLBACK_VALUE, 1, testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<FUNCTION_RESULT_ARG_INDEX>(FILTER_RESULT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, ToBool()).WillOnce(testing::Return(std::make_tuple(false, false)));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("not-boolean.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_006
 * @tc.desc: Verify Filter returns true when the callback accepts a name.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_006, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(FILTER_OBJECT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_call_function(ENV, FILTER_OBJECT, CALLBACK_VALUE, 1, testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<FUNCTION_RESULT_ARG_INDEX>(FILTER_RESULT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, ToBool()).WillOnce(testing::Return(std::make_tuple(true, true)));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_TRUE(filter.Filter("accepted.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_007
 * @tc.desc: Verify Filter returns false when the callback rejects a name.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_007, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(FILTER_OBJECT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_call_function(ENV, FILTER_OBJECT, CALLBACK_VALUE, 1, testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<FUNCTION_RESULT_ARG_INDEX>(FILTER_RESULT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, ToBool()).WillOnce(testing::Return(std::make_tuple(true, false)));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("rejected.txt"));
}

/**
 * @tc.name: FileFilterNapiMockTest_Filter_008
 * @tc.desc: Verify a callback failure is sticky across later filter names.
 * @tc.type: FUNC
 */
HWTEST_F(FileFilterNapiMockTest, FileFilterNapiMockTest_Filter_008, testing::ext::TestSize.Level1)
{
    ExpectConstructor();
    ExpectDestructor();
    ExpectOpenScope();
    ExpectCallbackPrelude();
    EXPECT_CALL(*mock_, napi_get_reference_value(ENV, FILTER_REFERENCE, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgPointee<VALUE_RESULT_ARG_INDEX>(FILTER_OBJECT), testing::Return(napi_ok)));
    EXPECT_CALL(*mock_, napi_call_function(ENV, FILTER_OBJECT, CALLBACK_VALUE, 1, testing::_, testing::_))
        .WillOnce(testing::Return(napi_pending_exception));
    ExpectCloseScope();
    LibN::NVal callback(ENV, CALLBACK_INPUT);
    FileFilterNapi filter(ENV, FILTER_OBJECT, callback);

    EXPECT_FALSE(filter.Filter("throws.txt"));
    EXPECT_FALSE(filter.Filter("never-called.txt"));
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
