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

#include "create_streamrw.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
namespace {
constexpr size_t MODULE_RESULT_ARG_INDEX = 2;
constexpr size_t PROPERTY_RESULT_ARG_INDEX = 3;
constexpr size_t INSTANCE_RESULT_ARG_INDEX = 4;

struct CreateStreamExpectation {
    const char *className;
    size_t argc;
    napi_status createStatus;
    napi_value result;
    napi_value firstArg = reinterpret_cast<napi_value>(0x2100);
    napi_value secondArg = reinterpret_cast<napi_value>(0x2200);
};
} // namespace

class CreateStreamRwMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
    }

    void TearDown() override
    {
        LibnMock::DisableMock();
    }

    void ExpectCreateStream(const CreateStreamExpectation &expectation)
    {
        napi_value module = reinterpret_cast<napi_value>(0x2300);
        napi_value constructor = reinterpret_cast<napi_value>(0x2400);
        auto mock = LibnMock::GetMock();
        EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*mock, GetArgc()).WillOnce(testing::Return(expectation.argc));
        EXPECT_CALL(*mock, napi_load_module(testing::_, testing::StrEq("@ohos.file.streamrw"), testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<MODULE_RESULT_ARG_INDEX>(module), testing::Return(napi_ok)));
        EXPECT_CALL(*mock,
            napi_get_named_property(testing::_, module, testing::StrEq(expectation.className), testing::_))
            .WillOnce(testing::DoAll(
                testing::SetArgPointee<PROPERTY_RESULT_ARG_INDEX>(constructor), testing::Return(napi_ok)));
        if (expectation.argc == NARG_CNT::ONE) {
            EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(expectation.firstArg));
        } else {
            EXPECT_CALL(*mock, GetArg(NARG_POS::FIRST)).WillOnce(testing::Return(expectation.firstArg));
            EXPECT_CALL(*mock, GetArg(NARG_POS::SECOND)).WillOnce(testing::Return(expectation.secondArg));
        }
        auto expectedArgs = testing::Truly([expectation](const napi_value *args) {
            if (args == nullptr || args[NARG_POS::FIRST] != expectation.firstArg) {
                return false;
            }
            return expectation.argc == NARG_CNT::ONE ||
                args[NARG_POS::SECOND] == expectation.secondArg;
        });
        EXPECT_CALL(*mock, napi_new_instance(testing::_, constructor, expectation.argc, expectedArgs, testing::_))
            .WillOnce(testing::DoAll(testing::SetArgPointee<INSTANCE_RESULT_ARG_INDEX>(expectation.result),
                testing::Return(expectation.createStatus)));
    }
};

/**
 * @tc.name: CreateStreamRwMockTest_Read_001
 * @tc.desc: Verify Read rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_));

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_001
 * @tc.desc: Verify Write rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_001, testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_));

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Read_002
 * @tc.desc: Verify Read creates a ReadStream with one argument.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_002, testing::ext::TestSize.Level1)
{
    napi_value expected = reinterpret_cast<napi_value>(0x2500);
    ExpectCreateStream({ "ReadStream", NARG_CNT::ONE, napi_ok, expected });

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}

/**
 * @tc.name: CreateStreamRwMockTest_Read_003
 * @tc.desc: Verify Read forwards two arguments to the ReadStream constructor.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_003, testing::ext::TestSize.Level1)
{
    napi_value expected = reinterpret_cast<napi_value>(0x2600);
    ExpectCreateStream({ "ReadStream", NARG_CNT::TWO, napi_ok, expected });

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}

/**
 * @tc.name: CreateStreamRwMockTest_Read_004
 * @tc.desc: Verify Read returns null when one-argument construction fails.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_004, testing::ext::TestSize.Level1)
{
    ExpectCreateStream({ "ReadStream", NARG_CNT::ONE, napi_generic_failure, nullptr });

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Read_005
 * @tc.desc: Verify Read returns null when two-argument construction fails.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_005, testing::ext::TestSize.Level1)
{
    ExpectCreateStream({ "ReadStream", NARG_CNT::TWO, napi_generic_failure, nullptr });

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_002
 * @tc.desc: Verify Write creates a WriteStream with one argument.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_002, testing::ext::TestSize.Level1)
{
    napi_value expected = reinterpret_cast<napi_value>(0x2700);
    ExpectCreateStream({ "WriteStream", NARG_CNT::ONE, napi_ok, expected });

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_003
 * @tc.desc: Verify Write forwards two arguments to the WriteStream constructor.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_003, testing::ext::TestSize.Level1)
{
    napi_value expected = reinterpret_cast<napi_value>(0x2800);
    ExpectCreateStream({ "WriteStream", NARG_CNT::TWO, napi_ok, expected });

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_004
 * @tc.desc: Verify Write returns null when one-argument construction fails.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_004, testing::ext::TestSize.Level1)
{
    ExpectCreateStream({ "WriteStream", NARG_CNT::ONE, napi_generic_failure, nullptr });

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_005
 * @tc.desc: Verify Write returns null when two-argument construction fails.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_005, testing::ext::TestSize.Level1)
{
    ExpectCreateStream({ "WriteStream", NARG_CNT::TWO, napi_generic_failure, nullptr });

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
}

/**
 * @tc.name: CreateStreamRwMockTest_Read_006
 * @tc.desc: Verify Read preserves the exact first constructor argument.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Read_006, testing::ext::TestSize.Level1)
{
    napi_value first = reinterpret_cast<napi_value>(0x3100);
    napi_value expected = reinterpret_cast<napi_value>(0x3200);
    ExpectCreateStream({ "ReadStream", NARG_CNT::ONE, napi_ok, expected, first });

    EXPECT_EQ(CreateStreamRw::Read(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}

/**
 * @tc.name: CreateStreamRwMockTest_Write_006
 * @tc.desc: Verify Write preserves both constructor arguments.
 * @tc.type: FUNC
 */
HWTEST_F(CreateStreamRwMockTest, CreateStreamRwMockTest_Write_006, testing::ext::TestSize.Level1)
{
    napi_value first = reinterpret_cast<napi_value>(0x3300);
    napi_value second = reinterpret_cast<napi_value>(0x3400);
    napi_value expected = reinterpret_cast<napi_value>(0x3500);
    ExpectCreateStream({ "WriteStream", NARG_CNT::TWO, napi_ok, expected, first, second });

    EXPECT_EQ(CreateStreamRw::Write(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), expected);
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
