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

#include "close_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "open_core.h"
#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
class CloseCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CloseCoreTest";
};

void CloseCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CloseCoreTest");
}

void CloseCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CloseCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void CloseCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CloseCoreTest_DoClose_001
 * @tc.desc: Test function of CloseCore::DoClose(fd) interface for FAILURE when fd is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreTest, CloseCoreTest_DoClose_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin CloseCoreTest_DoClose_001";

    auto ret = CloseCore::DoClose(-1);
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "CloseCoreTest-end CloseCoreTest_DoClose_001";
}

/**
 * @tc.name: CloseCoreTest_DoClose_002
 * @tc.desc: Test function of CloseCore::DoClose(fd) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreTest, CloseCoreTest_DoClose_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin CloseCoreTest_DoClose_002";

    auto path = testDir + "/CloseCoreTest_DoClose_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "CloseCoreTest_DoClose_002"));

    int32_t fd1 = open(path.c_str(), O_RDWR);
    if (fd1 < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd1 << ", errno: " << errno;
        ASSERT_TRUE(false);
    }

    auto ret = CloseCore::DoClose(fd1);
    EXPECT_TRUE(ret.IsSuccess());

    int32_t fd2 = open(path.c_str(), O_RDWR);
    if (fd2 < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd2 << ", errno: " << errno;
        ASSERT_TRUE(false);
    }
    EXPECT_EQ(fd1, fd2);

    ret = CloseCore::DoClose(fd2);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "CloseCoreTest-end CloseCoreTest_DoClose_002";
}

/**
 * @tc.name: CloseCoreTest_DoClose_003
 * @tc.desc: Test function of CloseCore::DoClose(file) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreTest, CloseCoreTest_DoClose_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin CloseCoreTest_DoClose_003";

    auto path = testDir + "/CloseCoreTest_DoClose_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "CloseCoreTest_DoClose_003"));

    auto fileRes = OpenCore::DoOpen(path);
    ASSERT_TRUE(fileRes.IsSuccess());
    FsFile *file = fileRes.GetData().value();
    ASSERT_NE(file, nullptr);

    auto ret = CloseCore::DoClose(file);
    EXPECT_TRUE(ret.IsSuccess());
    delete file;
    file = nullptr;

    GTEST_LOG_(INFO) << "CloseCoreTest-end CloseCoreTest_DoClose_003";
}

/**
 * @tc.name: CloseCoreTest_DoClose_004
 * @tc.desc: Test function of CloseCore::DoClose(file) interface for FAILURE to get fd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloseCoreTest, CloseCoreTest_DoClose_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin CloseCoreTest_DoClose_004";

    auto path = testDir + "/CloseCoreTest_DoClose_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "CloseCoreTest_DoClose_004"));

    auto fileRes = OpenCore::DoOpen(path);
    ASSERT_TRUE(fileRes.IsSuccess());
    FsFile *file = fileRes.GetData().value();
    ASSERT_NE(file, nullptr);

    auto ret = CloseCore::DoClose(file);
    EXPECT_TRUE(ret.IsSuccess());

    ret = CloseCore::DoClose(file);
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    delete file;
    file = nullptr;

    GTEST_LOG_(INFO) << "CloseCoreTest-end CloseCoreTest_DoClose_004";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS