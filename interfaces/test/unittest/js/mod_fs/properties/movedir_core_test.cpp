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

#include "movedir_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveDirCoreTest : public testing::Test {
public:
    static filesystem::path srcPath;
    static filesystem::path destPath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path MoveDirCoreTest::srcPath;
filesystem::path MoveDirCoreTest::destPath;

void MoveDirCoreTest::SetUpTestCase(void)
{
    srcPath = filesystem::temp_directory_path() / "src/";
    destPath = filesystem::temp_directory_path() / "dest/";
    std::filesystem::create_directory(srcPath);
    std::filesystem::create_directory(destPath);
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void MoveDirCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove_all(srcPath);
    filesystem::remove_all(destPath);
}

void MoveDirCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MoveDirCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0001
 * @tc.desc: Test function of DoMoveDir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0001";

    string src = srcPath.string() + "/test01";
    string dest = destPath.string();
    filesystem::create_directories(src);

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>());

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0001";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0002
 * @tc.desc: Test function of DoMoveDir() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0002";

    string src = srcPath.string() + "/test02";
    string dest = destPath.string();
    filesystem::create_directories(src);

    int invalidMode = DIRMODE_MAX + 1;
    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(invalidMode));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0002";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0003
 * @tc.desc: Test function of DoMoveDir() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0002";

    string src = srcPath.string() + "/test03";
    string dest = destPath.string();

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0003";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0004
 * @tc.desc: Test function of DoMoveDir() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0002";

    string src = srcPath.string();
    string dest = destPath.string() + "/test04";

    auto result = MoveDirCore::DoMoveDir(src, dest, optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_FALSE(result.errFiles.has_value());

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0004";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0005
 * @tc.desc: Test function of DoMoveDir() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0005";

    string src = "/data/local/test05/src/src/src/test05";
    string dest = destPath.string() + "/src";
    filesystem::create_directories(src);
    filesystem::create_directories(dest);

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test05/", destPath.string(), optional<int32_t>(DIRMODE_DIRECTORY_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900032);
    EXPECT_FALSE(result.errFiles.has_value());

    filesystem::remove_all("/data/local/test05");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0005";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0006
 * @tc.desc: Test function of DoMoveDir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0006";

    string src = "/data/local/test06/src/src/src/test06";
    string dest = destPath.string() + "/src";
    filesystem::create_directories(src);
    filesystem::create_directories(dest);

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test06/src", destPath.string(), optional<int32_t>(DIRMODE_DIRECTORY_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());

    filesystem::remove_all("/data/local/test06");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0006";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0007
 * @tc.desc: Test function of DoMoveDir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0007";

    filesystem::create_directories("/data/local/test07/src");
    filesystem::create_directories("/data/local/test07/dest");
    filesystem::path srcFile = "/data/local/test07/src/test_file.txt";
    ofstream(srcFile) << "Test content\n123\n456";
    filesystem::path destFile = "/data/local/test07/dest/test_file.txt";
    ofstream(destFile) << "Test content\ndest";

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test07/src/", "/data/local/test07/dest/", optional<int32_t>(DIRMODE_FILE_THROW_ERR));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    auto err = result.fsResult.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900015);
    EXPECT_TRUE(result.errFiles.has_value());

    filesystem::remove_all("/data/local/test07");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0007";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0008
 * @tc.desc: Test function of DoMoveDir() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0008";

    filesystem::create_directories("/data/local/test08/src");
    filesystem::create_directories("/data/local/test08/dest/test_file/test_file");
    filesystem::path srcFile = "/data/local/test08/src/test_file";
    ofstream(srcFile) << "Test content\n123\n456";

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test08/src/", "/data/local/test08/dest/test_file/", optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    filesystem::remove_all("/data/local/test08");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0008";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0009
 * @tc.desc: Test function of DoMoveDir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0009";

    filesystem::create_directories("/data/local/test09/src");
    filesystem::create_directories("/data/local/test09/dest");
    filesystem::path srcFile = "/data/local/test09/src/test_file.txt";
    ofstream(srcFile) << "Test content\n123\n456";
    filesystem::path destFile = "/data/local/test09/dest/test_file.txt";
    ofstream(destFile) << "Test content\ndest";

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test09/src/", "/data/local/test09/dest/", optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_TRUE(result.fsResult.IsSuccess());
    EXPECT_FALSE(result.errFiles.has_value());

    filesystem::remove_all("/data/local/test09");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0009";
}

/**
 * @tc.name: MoveDirCoreTest_DoMoveDir_0010
 * @tc.desc: Test function of DoMoveDir() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveDirCoreTest, MoveDirCoreTest_DoMoveDir_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveDirCoreTest-begin MoveDirCoreTest_DoMoveDir_0010";

    filesystem::create_directories("/data/local/test09/src/test_file.txt");
    filesystem::create_directories("/data/local/test09/dest");
    filesystem::path destFile = "/data/local/test09/dest/test_file.txt";
    ofstream(destFile) << "Test content\ndest";

    auto result = MoveDirCore::DoMoveDir(
        "/data/local/test09/src/", "/data/local/test09/dest/", optional<int32_t>(DIRMODE_FILE_REPLACE));

    EXPECT_FALSE(result.fsResult.IsSuccess());
    EXPECT_TRUE(result.errFiles.has_value());

    filesystem::remove_all("/data/local/test09");

    GTEST_LOG_(INFO) << "MoveDirCoreTest-end MoveDirCoreTest_DoMoveDir_0010";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test