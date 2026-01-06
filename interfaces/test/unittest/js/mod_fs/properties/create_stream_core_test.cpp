/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "create_stream_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;

class CreateStreamCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CreateStreamCoreTest";
};

void CreateStreamCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CreateStreamCoreTest");
}

void CreateStreamCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CreateStreamCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void CreateStreamCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CreateStreamCoreTest_DoCreateStream_001
 * @tc.desc: Test function of CreateStreamCore::DoCreateStream interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateStreamCoreTest, CreateStreamCoreTest_DoCreateStream_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateStreamCoreTest-begin CreateStreamCoreTest_DoCreateStream_001";

    string file = testDir + "/CreateStreamCoreTest_DoCreateStream_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(file, "content"));

    auto ret = CreateStreamCore::DoCreateStream(file, "r");

    ASSERT_TRUE(ret.IsSuccess());
    auto *stream = ret.GetData().value();
    ASSERT_NE(stream, nullptr);
    auto retClose = stream->Close();
    EXPECT_TRUE(retClose.IsSuccess());
    delete stream;
    stream = nullptr;

    GTEST_LOG_(INFO) << "CreateStreamCoreTest-end CreateStreamCoreTest_DoCreateStream_001";
}

/**
 * @tc.name: CreateStreamCoreTest_DoCreateStream_002
 * @tc.desc: Test function of CreateStreamCore::DoCreateStream interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateStreamCoreTest, CreateStreamCoreTest_DoCreateStream_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateStreamCoreTest-begin CreateStreamCoreTest_DoCreateStream_002";

    string file = testDir + "/CreateStreamCoreTest_DoCreateStream_002.txt";

    auto ret = CreateStreamCore::DoCreateStream(file, "sss");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "CreateStreamCoreTest-end CreateStreamCoreTest_DoCreateStream_002";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
