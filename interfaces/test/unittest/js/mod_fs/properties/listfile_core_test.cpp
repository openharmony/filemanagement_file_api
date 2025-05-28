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

#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include "listfile_core.h"

#define DIR_PATH "/data/test/ListFileCoreTest"
#define EMPTY_PATH "/data/test/Empty"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
namespace fs = std::filesystem;

static void create_file(const fs::path& path, const std::vector<uint8_t>& data, bool binary = true)
{
    fs::create_directories(path.parent_path()); // 确保目录存在
    std::ofstream file(path, binary ? std::ios::binary : std::ios::out);
    if (!file) {
        std::cerr << "创建文件失败: " << path << std::endl;
        return;
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    std::cout << "已创建文件: " << path << std::endl;
}

static void WriteBuffer(const string filename)
{
    const int targetSize = 1024;
    std::vector<char> buffer(targetSize, 0);
    std::ofstream file(filename, std::ios::binary);
    file.write(buffer.data(), buffer.size());
}

class ListFileCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        fs::create_directory(EMPTY_PATH);
        const std::vector<fs::path> directories = {
            "/data/test/ListFileCoreTest",                     // 单级目录
            "/data/test/ListFileCoreTest/level1",               // 二级目录
            "/data/test/ListFileCoreTest/level1/level2"         // 三级目录
        };

        // 为每个文件类型创建3个文件
        for (int i = 0; i < 3; ++i) {
            create_file(directories[i] / ("text_" + std::to_string(i + 1) + ".txt"),
                {'F', 'i', 'l', 'e', ' ', char('0' + i + 1)}, false);

            create_file(directories[i] / ("image_" + std::to_string(i + 1) + ".png"),
                {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A});

            create_file(directories[i] / ("photo_" + std::to_string(i + 1) + ".jpg"),
                {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46});

            create_file(directories[i] / ("data_" + std::to_string(i + 1) + ".data"),
                {0xAB, 0xCD, 0xEF, char(i), char(i + 1), char(i + 2)});
        }

        WriteBuffer("/data/test/ListFileCoreTest/text_1.txt");
    };
    static void TearDownTestCase()
    {
        rmdir(DIR_PATH);
        rmdir(EMPTY_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: DoListFileCoreTest_0001
* @tc.desc: Test function of DoListFileCore() interface for error suffix.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0001";
    FsListFileOptions opt;
    FsFileFilter filter;
    // filter.SetSuffix({"txt"}); // 无效后缀（缺少`.`）
    std::vector<std::string> suffixVector = {"txt"};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0001";
}

/**
* @tc.name: DoListFileCoreTest_0002
* @tc.desc: Test function of DoListFileCore() interface for error suffix.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0002";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> suffixVector = {".tx@t"};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0002";
}

/**
* @tc.name: DoListFileCoreTest_0003
* @tc.desc: Test function of DoListFileCore() interface for error listNum.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0003";
    FsListFileOptions opt;
    opt.listNum = -5;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0003";
}

/**
* @tc.name: DoListFileCoreTest_0004
* @tc.desc: Test function of DoListFileCore() interface for current suffix.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0004";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> suffixVector = {".txt"};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0004";
}

/**
* @tc.name: DoListFileCoreTest_0005
* @tc.desc: Test function of DoListFileCore() interface for no maching suffix .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0005";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> suffixVector = {".abc"};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_FALSE(files[0].empty());
    EXPECT_EQ(files[0], "level1");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0005";
}

/**
* @tc.name: DoListFileCoreTest_0006
* @tc.desc: Test function of DoListFileCore() interface for current display name .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0006";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> displayNameVector = {"text*.txt"};
    std::optional<std::vector<std::string>> optionalDisplayName = displayNameVector;
    filter.SetDisplayName(optionalDisplayName);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0006";
}

/**
* @tc.name: DoListFileCoreTest_0007
* @tc.desc: Test function of DoListFileCore() interface for current fileSizeOver .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0007";
    FsListFileOptions opt;
    FsFileFilter filter;
    filter.SetFileSizeOver(500);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "text_1.txt");
    EXPECT_EQ(files[1], "level1");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0007";
}

/**
* @tc.name: DoListFileCoreTest_0008
* @tc.desc: Test function of DoListFileCore() interface for displayMame = 0 .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0008";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> displayNameVector = {};
    std::optional<std::vector<std::string>> optionalDisplayName = displayNameVector;
    filter.SetDisplayName(optionalDisplayName);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0008";
}

/**
* @tc.name: DoListFileCoreTest_0009
* @tc.desc: Test function of DoListFileCore() interface for error fileSizeOver .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0009";
    FsListFileOptions opt;
    FsFileFilter filter;
    filter.SetFileSizeOver(-1);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0009";
}

/**
* @tc.name: DoListFileCoreTest_0010
* @tc.desc: Test function of DoListFileCore() interface for error lstModitiedAfter .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0010";
    FsListFileOptions opt;
    FsFileFilter filter;
    filter.SetLastModifiedAfter(-1);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0010";
}

/**
* @tc.name: DoListFileCoreTest_0011
* @tc.desc: Test function of DoListFileCore() interface for true recursion .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0011";
    FsListFileOptions opt;
    opt.recursion = true;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 12);
    EXPECT_EQ(files[0], "/text_1.txt");

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0011";
}

/**
* @tc.name: DoListFileCoreTest_0012
* @tc.desc: Test function of DoListFileCore() interface for empty directory .
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0012";
    auto result = ListFileCore::DoListFile(EMPTY_PATH);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_TRUE(files.empty());

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0012";
}

/**
* @tc.name: DoListFileCoreTest_0013
* @tc.desc: Test function of DoListFileCore() interface for current listNum.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0013";
    FsListFileOptions opt;
    opt.listNum = 3;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 3);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0013";
}

/**
* @tc.name: DoListFileCoreTest_0014
* @tc.desc: Test function of DoListFileCore() interface for no such file or directory.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0014";
    auto result = ListFileCore::DoListFile("/data/local/tmp/ascecedssw");
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0014";
}

/**
* @tc.name: DoListFileCoreTest_0015
* @tc.desc: Test function of DoListFileCore() interface for >256 length suffix.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0015";
    FsListFileOptions opt;
    FsFileFilter filter;

    const size_t targetLength = 257;
    std::string str = ".";
    for (size_t i = 1; i < targetLength; ++i) {
        str += std::to_string(i % 10);
    }

    std::vector<std::string> suffixVector = {str};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0015";
}

/**
* @tc.name: DoListFileCoreTest_0016
* @tc.desc: Test function of DoListFileCore() interface for empty suffix.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0016";
    FsListFileOptions opt;
    FsFileFilter filter;
    std::vector<std::string> suffixVector = {};
    std::optional<std::vector<std::string>> optionalSuffix = suffixVector;
    filter.SetSuffix(optionalSuffix);
    opt.filter = filter;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    EXPECT_FALSE(result.IsSuccess());

    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0016";
}

/**
* @tc.name: DoListFileCoreTest_0017
* @tc.desc: Test function of DoListFileCore() interface for 0 listNum.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ListFileCoreTest, DoListFileCoreTest_0017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileCoreTest-begin DoListFileCoreTest_0017";
    FsListFileOptions opt;
    opt.listNum = 0;

    auto result = ListFileCore::DoListFile(DIR_PATH, opt);
    ASSERT_TRUE(result.IsSuccess());

    auto files = result.GetData().value();
    EXPECT_EQ(files.size(), 5);

    GTEST_LOG_(INFO) << "ListFileCoreTest-end DoListFileCoreTest_0017";
}

}
}
}