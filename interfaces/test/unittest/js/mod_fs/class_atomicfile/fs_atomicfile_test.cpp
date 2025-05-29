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
#include "fs_atomicfile.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std;
namespace fs = std::filesystem;

string g_filePath = "/data/test/FsAtomicfileTest.txt";
string g_deleteFile = "/data/test/FsAtomicfileDelTest.txt";

class FsAtomicfileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FsAtomicfileTest::SetUpTestCase(void)
{
    ofstream tempfile(g_filePath);
    tempfile << "hello world";
    tempfile.close();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsAtomicfileTest::TearDownTestCase(void)
{
    filesystem::remove(g_filePath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsAtomicfileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsAtomicfileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsAtomicfileTest_GetPath_001
 * @tc.desc: Test function of FsAtomicFile::GetPath interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetPath_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    string path = stream->GetPath();
    EXPECT_EQ(path, g_filePath);

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetPath_001";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_001
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->GetBaseFile();
    EXPECT_TRUE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_001";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_002
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for path > PATH_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_002";

    size_t largeLength = static_cast<size_t>(PATH_MAX) + 1;
    string largeString(largeLength, 'a');

    auto ret = FsAtomicFile::Constructor(largeString);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->GetBaseFile();
    EXPECT_FALSE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_002";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_003
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for failed realpath.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_003";

    string path = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->GetBaseFile();
    EXPECT_FALSE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_003";
}

/**
 * @tc.name: FsAtomicfileTest_GetBaseFile_004
 * @tc.desc: Test function of FsAtomicFile::GetBaseFile interface for failed open.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_GetBaseFile_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_GetBaseFile_004";

    string path = "/data/test/aaaaaaaaaa.txt";

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->GetBaseFile();
    EXPECT_FALSE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_GetBaseFile_004";
}

/**
 * @tc.name: FsAtomicfileTest_StartWrite_001
 * @tc.desc: Test function of FsAtomicFile::StartWrite interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_StartWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_StartWrite_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    EXPECT_TRUE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_StartWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_StartWrite_002
 * @tc.desc: Test function of FsAtomicFile::StartWrite interface for no parent dir.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_StartWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_StartWrite_002";

    string path = "/data/local/tmp/test/test/test/test.txt";

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    EXPECT_FALSE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_StartWrite_002";
}

/**
 * @tc.name: FsAtomicfileTest_StartWrite_003
 * @tc.desc: Test function of FsAtomicFile::StartWrite interface for no permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_StartWrite_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_StartWrite_003";

    string path = "/sys/kernel/address_bits";

    auto ret = FsAtomicFile::Constructor(path);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    EXPECT_FALSE(retFl.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_StartWrite_003";
}

/**
 * @tc.name: FsAtomicfileTest_FinishWrite_001
 * @tc.desc: Test function of FsAtomicFile::FinishWrite interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_FinishWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_FinishWrite_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    ASSERT_TRUE(retFl.IsSuccess());
    string newPath = retFl.GetData().value();
    ofstream tempfile(newPath);
    tempfile << "hello world";
    tempfile.close();

    auto retFW = stream->FinishWrite();
    EXPECT_TRUE(retFW.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_FinishWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_FailWrite_001
 * @tc.desc: Test function of FsAtomicFile::FailWrite interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_FailWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_FailWrite_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    ASSERT_TRUE(retFl.IsSuccess());
    string newPath = retFl.GetData().value();
    ofstream tempfile(newPath);
    tempfile << "hello world";
    tempfile.close();

    auto retFW = stream->FailWrite();
    EXPECT_TRUE(retFW.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_FailWrite_001";
}

/**
 * @tc.name: FsAtomicfileTest_Delete_001
 * @tc.desc: Test function of FsAtomicFile::Delete interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_Delete_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_Delete_001";

    auto ret = FsAtomicFile::Constructor(g_deleteFile);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto retFl = stream->StartWrite();
    ASSERT_TRUE(retFl.IsSuccess());
    string newPath = retFl.GetData().value();
    ofstream tempfile(newPath);
    tempfile << "hello world";
    tempfile.close();

    auto retFW = stream->Delete();
    EXPECT_TRUE(retFW.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_Delete_001";
}

/**
 * @tc.name: FsAtomicfileTest_ReadFully_001
 * @tc.desc: Test function of FsAtomicFile::ReadFully interface for succ.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_ReadFully_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_ReadFully_001";

    auto ret = FsAtomicFile::Constructor(g_filePath);
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto result = stream->ReadFully();
    ASSERT_TRUE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_ReadFully_001";
}

/**
 * @tc.name: FsAtomicfileTest_ReadFully_002
 * @tc.desc: Test function of FsAtomicFile::ReadFully interface for valied path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicfileTest, FsAtomicfileTest_ReadFully_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicfileTest-begin FsAtomicfileTest_ReadFully_002";

    auto ret = FsAtomicFile::Constructor("aaaaaaaaaaaaaaaa");
    ASSERT_TRUE(ret.IsSuccess());

    shared_ptr<FsAtomicFile> stream(move(ret.GetData().value()));
    auto result = stream->ReadFully();
    ASSERT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsAtomicfileTest-end FsAtomicfileTest_ReadFully_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test