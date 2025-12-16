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

#include "truncate_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "ut_fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TruncateCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/TruncateCoreTest";
};

void TruncateCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "TruncateCoreTest");
}

void TruncateCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TruncateCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void TruncateCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_001
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FAILURE when fd is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_001";

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(-1, false);

    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_001";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_002
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for FAILURE when len is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_002";

    FileInfo fileInfo;
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(-1));
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1, false);

    auto res = TruncateCore::DoTruncate(fileInfo, len);
    EXPECT_FALSE(res.IsSuccess());

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_002";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_003
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with valid fd and default length (0).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_003";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_003.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_003 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, ""));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_003";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_004
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with valid fd and specific length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_004";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_004.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_004 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd
    auto len = fileContent.length() / 2;                         // Specify the half length

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, fileContent.substr(0, len)));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_004";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_005
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS - extend file with larger length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_005";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_005.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_005 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd
    auto len = fileContent.length() * 2;                         // Specify the double length

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::GetFileSize(fd), len);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_005";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_006
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with file filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_006";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_006.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    auto [succ, fileInfo] = GenerateFileInfoFromPath(filePath);
    ASSERT_TRUE(succ);

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, ""));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_006";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_007
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with file filePath and specific length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_007";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_007.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    auto [succ, fileInfo] = GenerateFileInfoFromPath(filePath);
    ASSERT_TRUE(succ);

    auto len = fileContent.length() / 2; // Specify the half length
    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, fileContent.substr(0, len)));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_007";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_008
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with non-existent file filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_008";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_008_non_existent.txt";

    auto [succ, fileInfo] = GenerateFileInfoFromPath(filePath);
    ASSERT_TRUE(succ);

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_008";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_009
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with directory filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_009";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_009";
    ASSERT_TRUE(FileUtils::CreateDirectories(filePath));

    auto [succ, fileInfo] = GenerateFileInfoFromPath(filePath);
    ASSERT_TRUE(succ);

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900019);
    EXPECT_EQ(err.GetErrMsg(), "Is a directory");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_009";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_010
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with read-only file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_010";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_010.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    FileInfo fileInfo;
    int fd = open(filePath.c_str(), O_RDONLY); // READ ONLY mode
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 failed to open file for reading: " << errno;
        ASSERT_TRUE(false);
    }
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_010";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_011
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with negative length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_011";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_011.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_011 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd
    auto len = -10;                                              // Specify the negative length

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_011";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_012
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with zero length using file filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_012";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_012.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    auto [succ, fileInfo] = GenerateFileInfoFromPath(filePath);
    ASSERT_TRUE(succ);

    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(0));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, ""));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_012";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_013
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with empty file filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_013";

    auto [succ, fileInfo] = GenerateFileInfoFromPath("");
    ASSERT_TRUE(succ);

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_013";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_014
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS - multiple truncate operations.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_014";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_014.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_014 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd

    auto len1 = 10;
    auto res1 = TruncateCore::DoTruncate(fileInfo, len1);

    EXPECT_TRUE(res1.IsSuccess());
    EXPECT_EQ(FileUtils::GetFileSize(fd), len1);

    auto len2 = 5;
    auto res2 = TruncateCore::DoTruncate(fileInfo, len2);

    EXPECT_TRUE(res2.IsSuccess());
    EXPECT_EQ(FileUtils::GetFileSize(fd), len2);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_014";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_015
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with null filePath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_015";

    FileInfo fileInfo;
    fileInfo.isPath = true;
    fileInfo.path = nullptr;

    auto res = TruncateCore::DoTruncate(fileInfo);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900013);
    EXPECT_EQ(err.GetErrMsg(), "Bad address");

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_015";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_016
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with exact content length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_016";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_016.txt";
    string fileContent = "Hello, this is a test file content for truncate testing!";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, fileContent));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_016 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd
    auto len = fileContent.length();

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, fileContent));

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_016";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_017
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS - truncate empty file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_017";

    string filePath = testDir + "/TruncateCoreTest_DoTruncate_017.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath));

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_017 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd); // Will auto close fd

    auto len = 10; // Specify the length
    auto res = TruncateCore::DoTruncate(fileInfo, len);

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::GetFileSize(fd), len);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_017";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test