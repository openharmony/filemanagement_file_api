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

#include "fs_atomicfile.h"

#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

class FsAtomicfileTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsAtomicfileTest";
};

void FsAtomicfileTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsAtomicfileTest");
}

void FsAtomicfileTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsAtomicfileTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsAtomicfileTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsAtomicfileTest_GetPath_001
 * @tc.desc: Test function of FsAtomicFile::GetPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetPath_001";

    string filePath = testDir + "/FsAtomicfileTest_GetPath_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, "content"));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    string path = file->GetPath();

    EXPECT_EQ(path, filePath);

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetPath_001";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_001
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_001";

    string filePath = testDir + "/FsAtomicfileTest_GetBaseFile_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, "content"));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->GetBaseFile();

    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsFile> baseFile(res.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(baseFile, nullptr);

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_001";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_002
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for FAILURE when path > PATH_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_002";

    size_t oversizedLen = static_cast<size_t>(PATH_MAX) + 1;
    std::allocator<char> alloc;
    constexpr std::__uninitialized_size_tag uninitTag {};
    string path(uninitTag, oversizedLen, std::move(alloc)); // Will only set string size, no actual memory allocated.

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->GetBaseFile();

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_002";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_003
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for FAILURE when path is not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_003";

    string path = testDir + "/FsAtomicfileTest_GetBaseFile_003_non_existent.txt";

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->GetBaseFile();

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_003";
}

/**
 * @tc.name: FsAtomicfileTest_StartWrite_001
 * @tc.desc: Test function of FsAtomicFile::StartWrite interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_StartWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_StartWrite_001";

    string filePath = testDir + "/FsAtomicfileTest_StartWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, "content"));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->StartWrite();

    ASSERT_TRUE(res.IsSuccess());
    auto newPath = res.GetData().value();
    EXPECT_NE(newPath, filePath);
    EXPECT_EQ(newPath.find(filePath, 0), 0);

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_StartWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_StartWrite_002
 * @tc.desc: Test function of FsAtomicFile::StartWrite interface for FAILURE when no parent dir.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_StartWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_StartWrite_002";

    string filePath = testDir + "/non_existent/FsAtomicfileTest_StartWrite_002.txt";

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->StartWrite();

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_StartWrite_002";
}

/**
 * @tc.name: FsAtomicfileTest_FinishWrite_001
 * @tc.desc: Test function of FsAtomicFile::FinishWrite interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_FinishWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_FinishWrite_001";

    string filePath = testDir + "/FsAtomicfileTest_FinishWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, "content"));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());

    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);
    auto startWriteRet = file->StartWrite();
    ASSERT_TRUE(startWriteRet.IsSuccess());
    string newPath = startWriteRet.GetData().value();
    auto content = "hello world";
    EXPECT_TRUE(FileUtils::DoCreateTextFile(newPath, content));

    auto res = file->FinishWrite();

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, content));
    EXPECT_EQ(file->entity->baseFileName, filePath);
    EXPECT_EQ(file->entity->newFileName, filePath.append(TEMP_FILE_SUFFIX));

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_FinishWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_FailWrite_001
 * @tc.desc: Test function of FsAtomicFile::FailWrite interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_FailWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_FailWrite_001";

    string filePath = testDir + "/FsAtomicfileTest_FailWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());

    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);
    auto startWriteRet = file->StartWrite();
    ASSERT_TRUE(startWriteRet.IsSuccess());
    string newPath = startWriteRet.GetData().value();
    auto content = "hello world";
    EXPECT_TRUE(FileUtils::DoCreateTextFile(newPath, content));

    auto res = file->FailWrite();

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(filePath), std::make_tuple(true, ""));
    EXPECT_FALSE(FileUtils::Exists(newPath));
    EXPECT_EQ(file->entity->baseFileName, filePath);
    EXPECT_EQ(file->entity->newFileName, filePath.append(TEMP_FILE_SUFFIX));

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_FailWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_Delete_001
 * @tc.desc: Test function of FsAtomicFile::Delete interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_Delete_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_Delete_001";

    string filePath = testDir + "/FsAtomicfileTest_Delete_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, "content"));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->Delete();

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(filePath));
    EXPECT_EQ(file->entity->baseFileName, "");
    EXPECT_EQ(file->entity->newFileName, "");

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_Delete_001";
}

/**
 * @tc.name: FsAtomicfileTest_ReadFully_001
 * @tc.desc: Test function of FsAtomicFile::ReadFully interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_ReadFully_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_ReadFully_001";

    string filePath = testDir + "/FsAtomicfileTest_ReadFully_001.txt";
    string content = "content";
    ASSERT_TRUE(FileUtils::CreateFile(filePath, content));

    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());

    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->ReadFully();

    ASSERT_TRUE(res.IsSuccess());
    auto data = move(res.GetData().value());
    ASSERT_NE(data, nullptr);
    string buf(reinterpret_cast<char*>(data->buffer), data->length);
    EXPECT_EQ(buf, content);

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_ReadFully_001";
}

/**
 * @tc.name: FsAtomicfileTest_ReadFully_002
 * @tc.desc: Test function of FsAtomicFile::ReadFully interface for FAILURE when path is not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_ReadFully_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_ReadFully_002";

    string filePath = testDir + "/FsAtomicfileTest_ReadFully_002_non_existent.txt";
    auto ret = FsAtomicFile::Constructor(filePath);
    ASSERT_TRUE(ret.IsSuccess());

    std::unique_ptr<FsAtomicFile> file(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);

    auto res = file->ReadFully();

    ASSERT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_ReadFully_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test