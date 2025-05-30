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

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "copy_dir_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyDirCoreTest : public testing::Test {
public:
    static filesystem::path srcPath;
    static filesystem::path destPath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void CreateTestFile(const filesystem::path& path, const string& content = "test") {
        ofstream file(path);
        file << content;
    }

    void CreateTestDirStructure() {
        filesystem::create_directories(srcPath / "subdir1");
        filesystem::create_directories(srcPath / "subdir2");
        CreateTestFile(srcPath / "file1.txt");
        CreateTestFile(srcPath / "subdir1" / "file2.txt");
        CreateTestFile(srcPath / "subdir2" / "file3.txt");
    }
};

filesystem::path CopyDirCoreTest::srcPath;
filesystem::path CopyDirCoreTest::destPath;

void CopyDirCoreTest::SetUpTestCase(void)
{
    srcPath = filesystem::temp_directory_path() / "src/";
    destPath = filesystem::temp_directory_path() / "dest/";
    std::filesystem::create_directory(srcPath);
    std::filesystem::create_directory(destPath);
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CopyDirCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove_all(srcPath);
    filesystem::remove_all(destPath);
}

void CopyDirCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyDirCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_001
 * @tc.desc: Test function of DoCopyDir() interface for SUCCESS with empty directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_001";

    string src = srcPath.string() + "/test01";
    string dest = destPath.string();
    filesystem::create_directories(src);

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_001";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_002
 * @tc.desc: Test function of DoCopyDir() interface for FAILED with invalid mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_002";

    string src = srcPath.string() + "/test02";
    string dest = destPath.string();
    filesystem::create_directories(src);

    int invalidMode = COPYMODE_MAX + 1;
    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(invalidMode));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_002";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_003
 * @tc.desc: Test function of DoCopyDir() interface for FAILED with non-existent source.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_003";

    string src = srcPath.string() + "/non_existent";
    string dest = destPath.string();

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_003";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_004
 * @tc.desc: Test function of DoCopyDir() interface for FAILED with invalid destination.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_004";

    string src = srcPath.string();
    string dest = destPath.string() + "/invalid_file.txt";
    filesystem::path(dest).remove_filename();
    filesystem::create_directories(filesystem::path(dest).parent_path());
    std::ofstream(dest).close(); // 创建文件而非目录

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_004";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_005
 * @tc.desc: Test function of DoCopyDir() interface for FAILED with same source and destination.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_005";

    string src = srcPath.string();
    string dest = srcPath.string();

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_005";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_006
 * @tc.desc: Test function of DoCopyDir() interface for SUCCESS with files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_006";

    string src = srcPath.string() + "/test06";
    string dest = destPath.string();
    filesystem::create_directories(src);
    CreateTestFile(src + "/file1.txt", "content1");
    CreateTestFile(src + "/file2.txt", "content2");

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_006";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_007
 * @tc.desc: Test function of DoCopyDir() interface for SUCCESS with subdirectories.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_007";

    string src = srcPath.string() + "/test07";
    string dest = destPath.string();
    filesystem::create_directories(src + "/subdir1");
    filesystem::create_directories(src + "/subdir2");
    CreateTestFile(src + "/subdir1/file1.txt", "sub1_content1");
    CreateTestFile(src + "/subdir2/file2.txt", "sub2_content2");

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_007";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_008
 * @tc.desc: Test function of DoCopyDir() interface for FAILED with existing files (throw error mode).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_008";

    string src = srcPath.string() + "/test08";
    string dest = destPath.string();
    filesystem::create_directories(src);
    CreateTestFile(src + "/file1.txt", "content1");
    
    string destDir = dest + "/" + filesystem::path(src).filename().string();
    filesystem::create_directories(destDir);
    CreateTestFile(destDir + "/file1.txt", "existing_content");

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(DIRMODE_FILE_COPY_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_008";
}

/**
 * @tc.name: CopyDirCoreTest_DoCopyDir_009
 * @tc.desc: Test function of DoCopyDir() interface for SUCCESS with existing files (overwrite mode).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(CopyDirCoreTest, CopyDirCoreTest_DoCopyDir_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDirCoreTest-begin CopyDirCoreTest_DoCopyDir_009";

    string src = srcPath.string() + "/test09";
    string dest = destPath.string();
    filesystem::create_directories(src);
    CreateTestFile(src + "/file1.txt", "content1");
    
    string destDir = dest + "/" + filesystem::path(src).filename().string();
    filesystem::create_directories(destDir);
    CreateTestFile(destDir + "/file1.txt", "existing_content");

    auto result = CopyDirCore::DoCopyDir(src, dest, optional<int32_t>(DIRMODE_FILE_COPY_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "CopyDirCoreTest-end CopyDirCoreTest_DoCopyDir_009";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test