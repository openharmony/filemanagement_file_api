/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mmap_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MmapCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/MmapCoreTest";
};

void MmapCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MmapCoreTest");
}

void MmapCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MmapCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void MmapCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_001
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when fd is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_001";

    auto result = MmapCore::DoMmap(-1, MappingMode::READ_ONLY, 0, 1024);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900008);

    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_001";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_002
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when mode is invalid (negative).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_002";

    string filepath = testDir + "/test_002.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, "test content");
    ASSERT_TRUE(success);

    auto result = MmapCore::DoMmap(fd, -1, 0, 1024);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_002";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_003
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when mode is invalid (>PRIVATE).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_003";

    string filepath = testDir + "/test_003.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, "test content");
    ASSERT_TRUE(success);

    auto result = MmapCore::DoMmap(fd, 100, 0, 1024);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_003";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_004
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when offset is negative.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_004";

    string filepath = testDir + "/test_004.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, "test content");
    ASSERT_TRUE(success);

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, -1, 1024);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_004";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_005
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when size is zero.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_005";

    string filepath = testDir + "/test_005.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, "test content");
    ASSERT_TRUE(success);

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 0);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_005";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_006
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with READ_ONLY mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_006";

    string content = "Hello, mmap test!";
    string filepath = testDir + "/test_006.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    size_t mapSize = content.size();
    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, mapSize);

    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_NE(mapping->GetEntity(), nullptr);
    EXPECT_EQ(mapping->GetEntity()->capacity, mapSize);
    EXPECT_EQ(mapping->GetEntity()->mode, MappingMode::READ_ONLY);
    EXPECT_TRUE(mapping->IsReadOnly());

    delete mapping;
    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_006";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_007
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with READ_WRITE mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_007";

    string content = "Hello, mmap test!";
    string filepath = testDir + "/test_007.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    size_t mapSize = content.size();
    auto result = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, mapSize);

    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_FALSE(mapping->IsReadOnly());

    delete mapping;
    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_007";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_008
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with PRIVATE mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_008";

    string content = "Hello, mmap test!";
    string filepath = testDir + "/test_008.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    size_t mapSize = content.size();
    auto result = MmapCore::DoMmap(fd, MappingMode::PRIVATE, 0, mapSize);

    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_FALSE(mapping->IsReadOnly());

    delete mapping;
    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_008";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_009
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with non-zero offset.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_009";

    string content = "Hello, mmap test with offset!";
    string filepath = testDir + "/test_009.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    off_t offset = 7;
    size_t mapSize = content.size() - offset;
    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, offset, mapSize);

    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_EQ(mapping->GetEntity()->offset, offset);

    delete mapping;
    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_009";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_010
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with file expansion.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_010";

    string content = "small";
    string filepath = testDir + "/test_010.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    size_t mapSize = 4096;
    auto result = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, mapSize);

    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);

    delete mapping;
    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_010";
}

/**
 * @tc.name: MmapCoreTest_DoMmap_011
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when file is a directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreTest, MmapCoreTest_DoMmap_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreTest-begin MmapCoreTest_DoMmap_011";

    int fd = open(testDir.c_str(), O_RDONLY | O_DIRECTORY);
    ASSERT_GE(fd, 0);

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    EXPECT_FALSE(result.IsSuccess());

    close(fd);
    GTEST_LOG_(INFO) << "MmapCoreTest-end MmapCoreTest_DoMmap_011";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
