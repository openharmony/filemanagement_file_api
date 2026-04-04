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

#include "listfile_core.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "i_file_filter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
namespace fs = std::filesystem;

struct FunctionTag {};

class TestFileFilter : public IFileFilter {
public:
    explicit TestFileFilter(bool returnValue) : returnValue_(returnValue) {}
    TestFileFilter(FunctionTag, std::function<bool(const std::string &)> filterFunc) : filterFunc_(filterFunc) {}

    bool Filter(const std::string &name) override
    {
        if (filterFunc_) {
            return filterFunc_(name);
        }
        return returnValue_;
    }

private:
    bool returnValue_ = true;
    std::function<bool(const std::string &)> filterFunc_ = nullptr;
};

class ListFileCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ListFileCoreTest";
    const string dataDir = testDir + "/data";
    const string emptyDir = testDir + "/emptyDir";
    const size_t totalRecursiveFiles = 12;
};

void ListFileCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ListFileCoreTest");
}

void ListFileCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ListFileCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";

    constexpr int32_t dataFileSize = 512;
    constexpr int32_t imageFileSize = 128;
    constexpr int32_t photoFileSize = 1024;

    string dirLevel1 = dataDir + "/level1";
    string dirLevel2 = dirLevel1 + "/level2";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(dirLevel1));
    ASSERT_TRUE(FileUtils::CreateDirectories(dirLevel2));
    ASSERT_TRUE(FileUtils::CreateDirectories(emptyDir));
    const std::vector<fs::path> directories = { dataDir, dirLevel1, dirLevel2 };
    // Create a different type of file in each directory, such as .txt, .data, .png,  etc.
    for (auto i = 0; i < directories.size(); i++) {
        auto txtFile = directories[i] / ("text_" + std::to_string(i + 1) + ".txt");
        auto imageFile = directories[i] / ("image_" + std::to_string(i + 1) + ".png");
        auto photoFile = directories[i] / ("photo_" + std::to_string(i + 1) + ".jpg");
        auto dataFile = directories[i] / ("data_" + std::to_string(i + 1) + ".data");
        ASSERT_TRUE(FileUtils::CreateFile(txtFile, "content"));
        ASSERT_TRUE(FileUtils::CreateFile(imageFile, imageFileSize));
        ASSERT_TRUE(FileUtils::CreateFile(photoFile, photoFileSize));
        ASSERT_TRUE(FileUtils::CreateFile(dataFile, dataFileSize));
    }
}

void ListFileCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_001
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when suffix is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_001";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> suffixVector = { "txt" };
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_001";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_002
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when suffix is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_002";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> suffixVector = { ".tx@t" };
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_002";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_003
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when listNum is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_003";

    FsListFileOptions opt;
    opt.listNum = -1;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_003";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_004
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when specifying a suffix.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_004";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> suffixVector = { ".txt" };
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 2);
    // Sort manually by dictionary
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files[0], "level1");
    EXPECT_EQ(files[1], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_004";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_005
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when specifying an unmatched suffix .
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_005";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> suffixVector = { ".abc" };
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "level1");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_005";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_006
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when specifying the display name.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_006";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> displayNameVector = { "text*.txt" };
    std::optional<std::vector<std::string>> optionalDisplayName = displayNameVector;
    filter.displayName = std::move(optionalDisplayName);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_006";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_007
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when specifying the fileSizeOver .
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_007";

    FsListFileOptions opt;
    FsFilter filter;
    filter.fileSizeOver = 500;
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 3);
    // Sort manually by dictionary
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files[0], "data_1.data");
    EXPECT_EQ(files[1], "level1");
    EXPECT_EQ(files[2], "photo_1.jpg");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_007";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_008
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when specifying none display name.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_008";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> displayNameVector = {};
    std::optional<std::vector<std::string>> optionalDisplayName = displayNameVector;
    filter.displayName = std::move(optionalDisplayName);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_008";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_009
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when fileSizeOver is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_009";

    FsListFileOptions opt;
    FsFilter filter;
    filter.fileSizeOver = -1;
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_009";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_010
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when lastModifiedAfter is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_010";

    FsListFileOptions opt;
    FsFilter filter;
    filter.lastModifiedAfter = -1;
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_010";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_011
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when recursion is true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_011";

    FsListFileOptions opt;
    opt.recursion = true;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    // Include 12 files, not include dir.
    ASSERT_EQ(files.size(), totalRecursiveFiles);
    // Sort manually by dictionary
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files[0], "/data_1.data");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_011";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_012
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when listing an empty directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_012";

    auto result = ListFileCore::DoListFile(emptyDir);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_TRUE(files.empty());

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_012";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_013
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when specifying listNum.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_013";

    FsListFileOptions opt;
    opt.listNum = 3;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 3);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_013";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_014
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when no such file or directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_014";

    auto path = dataDir + "/non_existent_dir";
    auto result = ListFileCore::DoListFile(path);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_014";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_015
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when suffix's length is over limit 256.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_015";

    FsListFileOptions opt;
    FsFilter filter;

    const size_t targetLength = 257;
    std::string suffix(targetLength - 1, 'x');
    suffix.insert(0, 1, '.');

    std::vector<std::string> suffixVector = { suffix };
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_015";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_016
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when suffix is empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_016";

    FsListFileOptions opt;
    FsFilter filter;
    std::vector<std::string> suffixVector = {};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.suffix = std::move(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_016";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_017
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when listNum is zero.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_017";

    FsListFileOptions opt;
    opt.listNum = 0;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_017";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_018
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches all files in
 * non-recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_018, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_018";

    FsListFileOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(files.begin(), files.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(files, expected);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_018";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_019
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches all files with
 * listNum limit in non-recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_019, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_019";

    FsListFileOptions opt;
    opt.recursion = false;
    opt.listNum = 2;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), opt.listNum);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_019";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_020
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches all files in
 * recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_020, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_020";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_020";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_021
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches all files with
 * listNum limit in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_021, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_021";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 5;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), opt.listNum);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_021";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_022
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches specific file in
 * non-recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_022, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_022";

    FsListFileOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name == "text_1.txt";
    });
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_022";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_023
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches files starting with
 * / in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_023, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_023";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name.find("/") == 0;
    });
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_023";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_024
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches files starting with
 * /level1 in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_024, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_024";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name.find("/level1") == 0;
    });
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    std::vector<std::string> expected = { "/level1/data_2.data", "/level1/image_2.png", "/level1/level2/data_3.data",
        "/level1/level2/image_3.png", "/level1/level2/photo_3.jpg", "/level1/level2/text_3.txt", "/level1/photo_2.jpg",
        "/level1/text_2.txt" };
    std::sort(files.begin(), files.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(files, expected);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_024";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_025
 * @tc.desc: Test function of ListFileCore::DoListFile interface for FAILURE when listNum is negative with fileFilter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_025, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_025";

    FsListFileOptions opt;
    opt.recursion = false;
    opt.listNum = -1;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_025";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_026
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches no files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_026, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_026";

    FsListFileOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(false);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 0);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_026";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_027
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when listNum exceeds actual file count in
 * recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_027, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_027";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 20;
    auto testFilter = std::make_shared<TestFileFilter>(true);
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_027";
}

/**
 * @tc.name: ListFileCoreTest_DoListFile_028
 * @tc.desc: Test function of ListFileCore::DoListFile interface for SUCCESS when fileFilter matches specific file in
 * subdirectory in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileCoreTest, ListFileCoreTest_DoListFile_028, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin ListFileCoreTest_DoListFile_028";

    FsListFileOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    auto testFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name == "/level1/text_2.txt";
    });
    opt.fileFilter = testFilter;

    auto result = ListFileCore::DoListFile(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "/level1/text_2.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end ListFileCoreTest_DoListFile_028";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS