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
#include <securec.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TruncateCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TruncateCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TruncateCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TruncateCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TruncateCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_001
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_001";
    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(-1);

    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_001";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_002
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_002";
    FileInfo fileInfo;
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(-1));
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    auto res = TruncateCore::DoTruncate(fileInfo, len);
    EXPECT_EQ(res.IsSuccess(), false);

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
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_003 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_003 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_003 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);

    auto res = TruncateCore::DoTruncate(fileInfo);

    struct stat st;
    int statResult = fstat(fd, &st);
    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_003 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 0);

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
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";
    
    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_004 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_004 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_004 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(10));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = fstat(fd, &st);
    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_004 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 10);
    

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
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_005 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_005 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_005 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(100));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = fstat(fd, &st);
    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_005 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 100);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_005";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_006
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with file path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_006";
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_006 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_006 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_006 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    close(fd);

    FileInfo fileInfo;
    size_t pathLen = filePath.length() + 1;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(pathLen);
    auto result = strcpy_s(fileInfo.path.get(), pathLen, filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "strcpy_s failed with error: " << result;
        ASSERT_TRUE(false);
    }

    auto res = TruncateCore::DoTruncate(fileInfo);

    struct stat st;
    int statResult = stat(filePath.c_str(), &st);

    result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_006 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 0);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_006";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_007
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with file path and specific length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_007";
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_007 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_007 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_007 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    close(fd);

    FileInfo fileInfo;
    size_t pathLen = filePath.length() + 1;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(pathLen);
    auto result = strcpy_s(fileInfo.path.get(), pathLen, filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "strcpy_s failed with error: " << result;
        ASSERT_TRUE(false);
    }
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(15));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = stat(filePath.c_str(), &st);

    result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_007 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 15);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_007";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_008
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with non-existent file path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_008";
    
    string nonExistentPath = "/data/storage/el2/NonExistentTruncateTestFile.txt";

    bool fileExists = (access(nonExistentPath.c_str(), F_OK) == 0);
    if (fileExists) {
        auto result = remove(nonExistentPath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_008 remove file failed! errno: " << errno;
        }
    }

    FileInfo fileInfo;
    size_t pathLen = nonExistentPath.length() + 1;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(pathLen);
    auto result = strcpy_s(fileInfo.path.get(), pathLen, nonExistentPath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "strcpy_s failed with error: " << result;
        ASSERT_TRUE(false);
    }

    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_008";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_009
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with directory path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_009";
    
    string dirPath = "/data/storage/el2/TruncateTestDir";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    auto result = mkdir(dirPath.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_009 mkdir failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    size_t pathLen = dirPath.length() + 1;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(pathLen);
    result = strcpy_s(fileInfo.path.get(), pathLen, dirPath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "strcpy_s failed with error: " << result;
        ASSERT_TRUE(false);
    }

    auto res = TruncateCore::DoTruncate(fileInfo);

    result = rmdir(dirPath.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_009 rmdir failed! errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), false);

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
    
    string filePath =  "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent =  "Hello, this is a test file content for truncate testing!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }

    close(fd);

    FileInfo fileInfo;
    int readFd = open(filePath.c_str(), O_RDONLY);
    if (readFd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 failed to open file for reading: " << errno;
        chmod(filePath.c_str(), 0644);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(readFd);

    auto res = TruncateCore::DoTruncate(fileInfo);

    close(readFd);

    chmod(filePath.c_str(), 0644);
    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_010 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), false);

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
    
    string filePath = "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent = "Test content for negative length test";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_011 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_011 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_011 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }
    
    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(-10));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_011 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_011";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_012
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for SUCCESS with zero length using file path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_012";
    
    string filePath = "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent = "Test content for zero length test";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_012 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_012 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_012 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }
    close(fd);

    FileInfo fileInfo;
    size_t pathLen = filePath.length() + 1;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(pathLen);
    auto strcpyResult = strcpy_s(fileInfo.path.get(), pathLen, filePath.c_str());
    if (strcpyResult != 0) {
        GTEST_LOG_(ERROR) << "strcpy_s failed with error: " << strcpyResult;
        EXPECT_EQ(strcpyResult, 0);
    }
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(0));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = stat(filePath.c_str(), &st);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_012 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 0);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_012";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_013
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with empty file path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_013";
    
    FileInfo fileInfo;
    fileInfo.isPath = true;
    fileInfo.path = std::make_unique<char[]>(1);
    fileInfo.path.get()[0] = '\0';

    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

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
    
    string filePath = "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent = "Hello, this is a test file content for multiple truncate operations!";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_014 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_014 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_014 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }
    
    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);

    std::optional<int64_t> len1 = std::make_optional(static_cast<int64_t>(10));
    auto res1 = TruncateCore::DoTruncate(fileInfo, len1);

    struct stat st1;
    int statResult1 = fstat(fd, &st1);

    std::optional<int64_t> len2 = std::make_optional(static_cast<int64_t>(5));
    auto res2 = TruncateCore::DoTruncate(fileInfo, len2);

    struct stat st2;
    int statResult2 = fstat(fd, &st2);

    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_014 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res1.IsSuccess(), true);
    EXPECT_EQ(res2.IsSuccess(), true);
    EXPECT_EQ(statResult1, 0);
    EXPECT_EQ(statResult2, 0);
    EXPECT_EQ(st1.st_size, 10);
    EXPECT_EQ(st2.st_size, 5);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_014";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_015
 * @tc.desc: Test function of TruncateCore::DoTruncate interface for FAILURE with null path.
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
    EXPECT_EQ(res.IsSuccess(), false);

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
    
    string filePath = "/data/storage/el2/TruncateCoreTestFile.txt";
    string fileContent = "Exact length test";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_016 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }
    
    ssize_t written = write(fd, fileContent.c_str(), fileContent.length());
    if (written != static_cast<ssize_t>(fileContent.length())) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_016 write failed! errno: " << errno;
        close(fd);
        auto result = remove(filePath.c_str());
        if (result != 0) {
            GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_016 remove file failed! errno: " << errno;
        }
        ASSERT_TRUE(false);
    }
    
    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(fileContent.length()));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = fstat(fd, &st);
    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_016 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(written, static_cast<ssize_t>(fileContent.length()));
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, fileContent.length());

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
    
    string filePath = "/data/storage/el2/TruncateCoreTestFile.txt";

    int fd = open(filePath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_017 create failed! errno: " << errno;
        ASSERT_TRUE(false);
    }

    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(fd);
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(100));

    auto res = TruncateCore::DoTruncate(fileInfo, len);

    struct stat st;
    int statResult = fstat(fd, &st);
    close(fd);

    auto result = remove(filePath.c_str());
    if (result != 0) {
        GTEST_LOG_(ERROR) << "TruncateCoreTest_DoTruncate_017 remove file failed! errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_EQ(statResult, 0);
    EXPECT_EQ(st.st_size, 100);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_017";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test