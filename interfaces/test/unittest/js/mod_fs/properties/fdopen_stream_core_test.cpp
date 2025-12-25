/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "fdopen_stream_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;

class FdopenStreamCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FdopenStreamCoreTest";
};

void FdopenStreamCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FdopenStreamCoreTest");
}

void FdopenStreamCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FdopenStreamCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FdopenStreamCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FdopenStreamCoreTest_DoFdopenStream_001
 * @tc.desc: Test function of FdopenStreamCore::DoFdopenStream interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FdopenStreamCoreTest, FdopenStreamCoreTest_DoFdopenStream_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin FdopenStreamCoreTest_DoFdopenStream_001";

    string path = testDir + "/FdopenStreamCoreTest_DoFdopenStream_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    int32_t fd = open(path.c_str(), O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        ASSERT_TRUE(false);
    }

    auto ret = FdopenStreamCore::DoFdopenStream(fd, "r");
    ASSERT_TRUE(ret.IsSuccess());

    auto stream = ret.GetData().value();
    ASSERT_NE(stream, nullptr);
    auto retClose = stream->Close();
    EXPECT_TRUE(retClose.IsSuccess());

    close(fd);

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end FdopenStreamCoreTest_DoFdopenStream_001";
}

/**
 * @tc.name: FdopenStreamCoreTest_DoFdopenStream_002
 * @tc.desc: Test function of FdopenStreamCore::DoFdopenStream interface for FAILURE when fd < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FdopenStreamCoreTest, FdopenStreamCoreTest_DoFdopenStream_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin FdopenStreamCoreTest_DoFdopenStream_002";

    auto ret = FdopenStreamCore::DoFdopenStream(-1, "r");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end FdopenStreamCoreTest_DoFdopenStream_002";
}

/**
 * @tc.name: FdopenStreamCoreTest_DoFdopenStream_003
 * @tc.desc: Test function of FdopenStreamCore::DoFdopenStream interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FdopenStreamCoreTest, FdopenStreamCoreTest_DoFdopenStream_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin FdopenStreamCoreTest_DoFdopenStream_003";

    string path = testDir + "/FdopenStreamCoreTest_DoFdopenStream_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    int32_t fd = open(path.c_str(), O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        ASSERT_TRUE(false);
    }

    auto ret = FdopenStreamCore::DoFdopenStream(fd, "sssssss");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    close(fd);

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end FdopenStreamCoreTest_DoFdopenStream_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS