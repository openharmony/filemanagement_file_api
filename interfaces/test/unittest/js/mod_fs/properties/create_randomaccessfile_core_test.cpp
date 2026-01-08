/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "create_randomaccessfile_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CreateRandomAccessFileCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CreateRandomAccessFileCoreTest";
};

void CreateRandomAccessFileCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CreateRandomAccessFileCoreTest");
}

void CreateRandomAccessFileCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CreateRandomAccessFileCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void CreateRandomAccessFileCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FAILURE when mode is
 * invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001";

    string path = testDir + "/CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001.txt";
    int32_t mode = -1;
    optional<RandomAccessFileOptions> options = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, options);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FAILURE when start
 * option is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002";

    string path = testDir + "/CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002.txt";
    int32_t mode = 0;
    RandomAccessFileOptions opts;
    opts.start = -1;
    opts.end = 1;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, opts);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FAILURE when end option
 * is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003";

    string path = testDir + "/CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003.txt";
    int32_t mode = 0;
    RandomAccessFileOptions opts;
    opts.start = 0;
    opts.end = -1;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, opts);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003";
}

/**'
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FAILURE when fd is
 * invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004";

    int fd = -1;
    optional<RandomAccessFileOptions> opts = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(fd, opts);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004";
}

/**'
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FAILURE when path is
 * empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005";

    string path = "";
    int32_t mode = 0;
    optional<RandomAccessFileOptions> options = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, options);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006";

    string file = testDir + "/CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006.txt";
    int fd = open(file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_GT(fd, -1);

    optional<RandomAccessFileOptions> opts = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(fd, opts);

    close(fd);
    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsRandomAccessFile> raf(res.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(raf, nullptr);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test