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

#include "listfile_ext_core.h"

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

class ListFileExtCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/ListFileExtCoreTest";
    const string dataDir = testDir + "/data";
    const string emptyDir = testDir + "/emptyDir";
    const size_t totalRecursiveFiles = 12;
};

void ListFileExtCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ListFileExtCoreTest");
}

void ListFileExtCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ListFileExtCoreTest::SetUp()
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

void ListFileExtCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_001";

    auto result = ListFileExtCore::DoListFileExt(dataDir, std::nullopt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_001";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_002";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = std::nullopt;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_002";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_003";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = -1;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_003";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_004";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_004";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_005";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 3;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 3);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_005";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_006";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), totalRecursiveFiles);
    std::sort(files.begin(), files.end());
    EXPECT_EQ(files[0], "/data_1.data");

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_006";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_007";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 5;
    opt.fileFilter = nullptr;

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_007";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_008";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(files.begin(), files.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(files, expected);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_008";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_009";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 2;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 2);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_009";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_010";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(false);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 0);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_010";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_011";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_011";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_012";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 5;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_012";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_013";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name == "text_1.txt";
    });

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_013";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_014";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name.find("/") == 0;
    });

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_014";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_015";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name.find("/level1") == 0;
    });

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    std::vector<std::string> expected = { "/level1/data_2.data", "/level1/image_2.png", "/level1/level2/data_3.data",
        "/level1/level2/image_3.png", "/level1/level2/photo_3.jpg", "/level1/level2/text_3.txt", "/level1/photo_2.jpg",
        "/level1/text_2.txt" };
    std::sort(files.begin(), files.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(files, expected);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_015";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_016";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 0;
    opt.fileFilter = std::make_shared<TestFileFilter>(FunctionTag {}, [](const std::string &name) {
        return name == "/level1/text_2.txt";
    });

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "/level1/text_2.txt");

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_016";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_017";

    ListFileExtOptions opt;
    opt.recursion = false;
    opt.listNum = -1;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_017";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_018, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_018";

    ListFileExtOptions opt;
    opt.recursion = true;
    opt.listNum = 20;
    opt.fileFilter = std::make_shared<TestFileFilter>(true);

    auto result = ListFileExtCore::DoListFileExt(dataDir, opt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    ASSERT_EQ(files.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_018";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_019, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_019";

    auto result = ListFileExtCore::DoListFileExt(emptyDir, std::nullopt);

    ASSERT_TRUE(result.IsSuccess());
    auto files = result.GetData().value();
    EXPECT_TRUE(files.empty());

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_019";
}

HWTEST_F(ListFileExtCoreTest, ListFileExtCoreTest_DoListFileExt_020, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileExtCoreTest-begin DoListFileExt_020";

    auto path = dataDir + "/non_existent_dir";
    auto result = ListFileExtCore::DoListFileExt(path, std::nullopt);

    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "ListFileExtCoreTest-end DoListFileExt_020";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
