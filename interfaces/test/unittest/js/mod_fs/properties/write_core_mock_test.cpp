/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "write_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class WriteCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WriteCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "WriteCoreMockTest");
    UvFsMock::EnableMock();
}

void WriteCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WriteCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void WriteCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: WriteCoreMockTest_DoWrite_001
 * @tc.desc: Test function of WriteCore::DoWrite(StringBuffer) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreMockTest, WriteCoreMockTest_DoWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreMockTest-begin WriteCoreMockTest_DoWrite_001";

    int32_t fd = 1;
    string buffer;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto res = WriteCore::DoWrite(fd, buffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "WriteCoreMockTest-end WriteCoreMockTest_DoWrite_001";
}

/**
 * @tc.name: WriteCoreMockTest_DoWrite_002
 * @tc.desc: Test function of WriteCore::DoWrite(StringBuffer) interface for FAILURE when uv_fs_write fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreMockTest, WriteCoreMockTest_DoWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreMockTest-begin WriteCoreMockTest_DoWrite_002";

    int32_t fd = 1;
    string buffer;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = WriteCore::DoWrite(fd, buffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "WriteCoreMockTest-end WriteCoreMockTest_DoWrite_002";
}

/**
 * @tc.name: WriteCoreMockTest_DoWrite_003
 * @tc.desc: Test function of WriteCore::DoWrite(ArrayBuffer) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreMockTest, WriteCoreMockTest_DoWrite_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreMockTest-begin WriteCoreMockTest_DoWrite_003";

    int32_t fd = 1;
    ArrayBuffer buffer(nullptr, 1);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto res = WriteCore::DoWrite(fd, buffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "WriteCoreMockTest-end WriteCoreMockTest_DoWrite_003";
}

/**
 * @tc.name: WriteCoreMockTest_DoWrite_004
 * @tc.desc: Test function of WriteCore::DoWrite(ArrayBuffer) interface for FAILURE when uv_fs_write fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreMockTest, WriteCoreMockTest_DoWrite_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreMockTest-begin WriteCoreMockTest_DoWrite_004";

    int32_t fd = 1;
    ArrayBuffer buffer(nullptr, 1);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = WriteCore::DoWrite(fd, buffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "WriteCoreMockTest-end WriteCoreMockTest_DoWrite_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test