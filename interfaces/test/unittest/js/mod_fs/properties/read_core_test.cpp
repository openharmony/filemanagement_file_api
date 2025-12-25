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

#include <gtest/gtest.h>
#include <sys/prctl.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void ReadCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ReadCoreTest");
}

void ReadCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ReadCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ReadCoreTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadCoreTest_DoRead_001
 * @tc.desc: Test function of ReadCore::DoRead interface for FAILURE when fd is invalid (negative value).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreTest, ReadCoreTest_DoRead_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreTest-begin ReadCoreTest_DoRead_001";

    int32_t fd = -1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);

    auto res = ReadCore::DoRead(fd, arrayBuffer);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadCoreTest-end ReadCoreTest_DoRead_001";
}

/**
 * @tc.name: ReadCoreTest_DoRead_003
 * @tc.desc: Test function of ReadCore::DoRead interface for FAILURE when offset is invalid (negative value).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreTest, ReadCoreTest_DoRead_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreTest-begin ReadCoreTest_DoRead_003";

    int32_t fd = 1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);
    optional<ReadOptions> options = std::make_optional<ReadOptions>();
    options->offset = std::make_optional<int64_t>(-1);

    auto res = ReadCore::DoRead(fd, arrayBuffer, options);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ReadCoreTest-end ReadCoreTest_DoRead_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test