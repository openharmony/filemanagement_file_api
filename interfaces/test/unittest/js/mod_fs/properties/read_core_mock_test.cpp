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

#include "read_core.h"

#include <climits>
#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ReadCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "ReadCoreMockTest");
    UvFsMock::EnableMock();
}

void ReadCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ReadCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ReadCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadCoreMockTest_DoRead_001
 * @tc.desc: Test function of ReadCore::DoRead interface for FAILURE when uv_fs_read fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreMockTest, ReadCoreMockTest_DoRead_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreMockTest-begin ReadCoreMockTest_DoRead_001";

    int32_t fd = 1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = ReadCore::DoRead(fd, arrayBuffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "ReadCoreMockTest-end ReadCoreMockTest_DoRead_001";
}

/**
 * @tc.name: ReadCoreMockTest_DoRead_002
 * @tc.desc: Test function of ReadCore::DoRead interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreMockTest, ReadCoreMockTest_DoRead_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreMockTest-begin ReadCoreMockTest_DoRead_002";

    int32_t fd = 1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(1));

    auto res = ReadCore::DoRead(fd, arrayBuffer);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "ReadCoreMockTest-end ReadCoreMockTest_DoRead_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test