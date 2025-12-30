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

#include "open_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class OpenCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/OpenCoreTest";
};

void OpenCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "OpenCoreTest");
}

void OpenCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void OpenCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void OpenCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_001
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when mode is invalid (negative mode).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_001";

    string path = testDir + "/OpenCoreTest_DoOpen_001.txt";
    int32_t mode = -1;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_001";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_002
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when mode is invalid (mode conflict).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_002";

    string path = testDir + "/OpenCoreTest_DoOpen_002.txt";
    int32_t mode = O_WRONLY | O_RDWR;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_002";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_003
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when media URL is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_003";

    string path = "file://media/OpenCoreTest_DoOpen_003_non_existent.jpg";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_003";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_004
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when docs URL is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_004";

    string path = "file://docs/OpenCoreTest_DoOpen_004_non_existent.pdf";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_004";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_005
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when content URL is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_005";

    string path = "content://com.example.provider/OpenCoreTest_DoOpen_005_non_existent.txt";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_005";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_006
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when datashare URL is invalid (not a remote URL).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_006";

    string path = "datashare:///media/OpenCoreTest_DoOpen_006.jpg";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_006";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_007
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when URL is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_007";

    string path = "invalid://fakePath/OpenCoreTest_DoOpen_007.txt";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_007";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_008
 * @tc.desc: Test function of OpenCore::DoOpen interface for FAILURE when URL is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_008";

    string path = "internal://fakePath/OpenCoreTest_DoOpen_008.txt";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_008";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test