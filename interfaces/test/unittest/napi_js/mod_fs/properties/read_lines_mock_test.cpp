/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "read_lines.h"

#include <cstring>
#include <fcntl.h>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;

class ReadLinesMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void ReadLinesMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ReadLinesMockTest");
}

void ReadLinesMockTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ReadLinesMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    LibnMock::EnableMock();
    errno = 0;
}

void ReadLinesMockTest::TearDown()
{
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadLinesMockTest_Sync_001
 * @tc.desc: Test function of ReadLinesSync interface for FAILURE when ARGS ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesMockTest, ReadLinesMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesMockTest-begin ReadLinesMockTest_Sync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = ReadLines::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "ReadLinesMockTest-end ReadLinesMockTest_Sync_001";
}

/**
 * @tc.name: ReadLinesMockTest_Sync_002
 * @tc.desc: Test function of ReadLinesSync interface for FAILURE when Analyze args ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadLinesMockTest, ReadLinesMockTest_Sync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLinesMockTest-begin ReadLinesMockTest_Sync_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    size_t strLen = 10;
    auto strPtr = make_unique<char[]>(strLen);
    tuple<bool, unique_ptr<char[]>, size_t> isStr = { false, move(strPtr), strLen };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(isStr)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = ReadLines::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "ReadLinesMockTest-end ReadLinesMockTest_Sync_002";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test