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

#include "copy_core.h"

#include <fcntl.h>
#include <filesystem>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static const string testDir;
    static const string srcDir;
    static const string destDir;
    static const string srcFile;
    static const string destFile;
};

const string CopyCoreTest::testDir = "/data/test";
const string CopyCoreTest::srcDir = testDir + "/src";
const string CopyCoreTest::destDir = testDir + "/dest";
const string CopyCoreTest::srcFile = srcDir + "/src.txt";
const string CopyCoreTest::destFile = destDir + "/dest.txt";

void CopyCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mkdir(testDir.c_str(), 0755);
    mkdir(srcDir.c_str(), 0755);
    mkdir(destDir.c_str(), 0755);
    int32_t fd = open(srcFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd >= 0) {
        close(fd);
    }
}

void CopyCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    remove(srcFile.c_str());
    rmdir(srcDir.c_str());
    rmdir(destDir.c_str());
    rmdir(testDir.c_str());
}

void CopyCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    remove(destFile.c_str());
}

/**
 * @tc.name: CopyCoreTest_IsValidUri_001
 * @tc.desc: Test function of CopyCore::IsValidUri interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsValidUri_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsValidUri_001";

    string validUri = "file://data/test/file.txt";
    auto res = CopyCore::IsValidUri(validUri);
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsValidUri_001";
}

/**
 * @tc.name: CopyCoreTest_IsValidUri_002
 * @tc.desc: Test function of CopyCore::IsValidUri interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsValidUri_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsValidUri_002";

    string invalidUri = "invalid://data/test/file.txt";
    auto res = CopyCore::IsValidUri(invalidUri);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsValidUri_002";
}

/**
 * @tc.name: CopyCoreTest_IsRemoteUri_001
 * @tc.desc: Test function of CopyCore::IsRemoteUri interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsRemoteUri_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsRemoteUri_001";

    string remoteUri = "file://data/test/file.txt?networkid=123";
    auto res = CopyCore::IsRemoteUri(remoteUri);
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsRemoteUri_001";
}

/**
 * @tc.name: CopyCoreTest_IsRemoteUri_002
 * @tc.desc: Test function of CopyCore::IsRemoteUri interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsRemoteUri_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsRemoteUri_002";

    string localUri = "file://data/test/file.txt";
    auto res = CopyCore::IsRemoteUri(localUri);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsRemoteUri_002";
}

/**
 * @tc.name: CopyCoreTest_IsDirectory_001
 * @tc.desc: Test function of CopyCore::IsDirectory interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsDirectory_001";

    auto res = CopyCore::IsDirectory(srcDir);
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsDirectory_001";
}

/**
 * @tc.name: CopyCoreTest_IsDirectory_002
 * @tc.desc: Test function of CopyCore::IsDirectory interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsDirectory_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsDirectory_002";

    auto res = CopyCore::IsDirectory(srcFile);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsDirectory_002";
}

/**
 * @tc.name: CopyCoreTest_IsFile_001
 * @tc.desc: Test function of CopyCore::IsFile interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsFile_001";

    auto res = CopyCore::IsFile(srcFile);
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsFile_001";
}

/**
 * @tc.name: CopyCoreTest_IsFile_002
 * @tc.desc: Test function of CopyCore::IsFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsFile_002";

    auto res = CopyCore::IsFile(srcDir);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsFile_002";
}

/**
 * @tc.name: CopyCoreTest_IsMediaUri_001
 * @tc.desc: Test function of CopyCore::IsMediaUri interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_IsMediaUri_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_IsMediaUri_001";

    auto res = CopyCore::IsMediaUri(srcFile);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsMediaUri_001";
}

/**
 * @tc.name: CopyCoreTest_GetFileSize_001
 * @tc.desc: Test function of CopyCore::GetFileSize interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetFileSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetFileSize_001";

    auto [err, size] = CopyCore::GetFileSize(srcFile);
    EXPECT_EQ(err, ERRNO_NOERR);
    EXPECT_GE(size, 0);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetFileSize_001";
}

/**
 * @tc.name: CopyCoreTest_GetFileSize_002
 * @tc.desc: Test function of CopyCore::GetFileSize interface for failure.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetFileSize_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetFileSize_002";

    string nonExistentFile = "/data/test/non_existent.txt";
    auto [err, size] = CopyCore::GetFileSize(nonExistentFile);
    EXPECT_NE(err, ERRNO_NOERR);
    EXPECT_EQ(size, 0);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetFileSize_002";
}

/**
 * @tc.name: CopyCoreTest_CheckOrCreatePath_001
 * @tc.desc: Test function of CopyCore::CheckOrCreatePath interface for existing file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CheckOrCreatePath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CheckOrCreatePath_001";

    auto res = CopyCore::CheckOrCreatePath(srcFile);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CheckOrCreatePath_001";
}

/**
 * @tc.name: CopyCoreTest_CheckOrCreatePath_002
 * @tc.desc: Test function of CopyCore::CheckOrCreatePath interface for creating new file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CheckOrCreatePath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CheckOrCreatePath_002";

    string newFile = destDir + "/new_file.txt";
    auto res = CopyCore::CheckOrCreatePath(newFile);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsFile(newFile));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CheckOrCreatePath_002";
}

/**
 * @tc.name: CopyCoreTest_MakeDir_001
 * @tc.desc: Test function of CopyCore::MakeDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_MakeDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_MakeDir_001";

    string newDir = destDir + "/new_dir";
    auto res = CopyCore::MakeDir(newDir);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(newDir));

    rmdir(newDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_MakeDir_001";
}

/**
 * @tc.name: CopyCoreTest_MakeDir_002
 * @tc.desc: Test function of CopyCore::MakeDir interface for existing directory.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_MakeDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_MakeDir_002";

    auto res = CopyCore::MakeDir(srcDir);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_MakeDir_002";
}

/**
 * @tc.name: CopyCoreTest_MakeDir_003
 * @tc.desc: Test function of CopyCore::MakeDir interface for invalid path.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 2
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_MakeDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_MakeDir_003";

    string invalidPath = "/invalid/path/dir";
    auto res = CopyCore::MakeDir(invalidPath);
    EXPECT_NE(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_MakeDir_003";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_001
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_001";

    string srcFile = "invalid://data/test/src.txt";

    auto res = CopyCore::ValidParams(srcFile, destFile);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_001";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_002
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_002";

    string destFile = "invalid://data/test/dest.txt";

    auto res = CopyCore::ValidParams(srcFile, destFile);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_002";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_003
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_003";

    string src = "file://data/test/src.txt";
    string dest = "file://data/test/dest.txt";

    auto res = CopyCore::ValidParams(src, dest);
    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_003";
}

/**
 * @tc.name: CopyCoreTest_CreateFileInfos_001
 * @tc.desc: Test function of CopyCore::CreateFileInfos interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CreateFileInfos_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CreateFileInfos_001";

    optional<CopyOptions> options = std::make_optional<CopyOptions>();

    auto [errCode, infos] = CopyCore::CreateFileInfos(srcFile, destFile, options);
    EXPECT_EQ(errCode, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CreateFileInfos_001";
}

/**
 * @tc.name: CopyCoreTest_CopySubDir_001
 * @tc.desc: Test function of CopyCore::CopySubDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopySubDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopySubDir_001";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), 0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd >= 0) {
        close(fd);
    }

    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    string destSubFile = destSubDir + "/sub_file.txt";
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(destSubDir));
    EXPECT_TRUE(CopyCore::IsFile(destSubFile));

    remove(subFile.c_str());
    rmdir(subDir.c_str());
    remove(destSubFile.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopySubDir_001";
}

/**
 * @tc.name: CopyCoreTest_CopySubDir_002
 * @tc.desc: Test function of CopyCore::CopySubDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopySubDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopySubDir_002";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), 0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd >= 0) {
        close(fd);
    }

    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    EXPECT_EQ(res, errno);

    remove(subFile.c_str());
    rmdir(subDir.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopySubDir_002";
}

/**
 * @tc.name: CopyCoreTest_RecurCopyDir_001
 * @tc.desc: Test function of CopyCore::RecurCopyDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_RecurCopyDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_RecurCopyDir_001";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), 0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd >= 0) {
        close(fd);
    }

    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::RecurCopyDir(srcDir, destDir, infos);
    string destSubFile = destSubDir + "/sub_file.txt";
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(destSubDir));
    EXPECT_TRUE(CopyCore::IsFile(destSubDir + "/sub_file.txt"));

    remove(subFile.c_str());
    rmdir(subDir.c_str());
    remove(destSubFile.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_RecurCopyDir_001";
}

/**
 * @tc.name: CopyCoreTest_CopyDirFunc_001
 * @tc.desc: Test function of CopyCore::CopyDirFunc interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopyDirFunc_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopyDirFunc_001";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), 0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd >= 0) {
        close(fd);
    }
    
    string destSubDir = destDir + "/src/sub_dir";
    string destSubFile = destSubDir + "/sub_file.txt";
    string destSrcDir = destDir + "/src";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::CopyDirFunc(srcDir, destDir, infos);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_EQ(CopyCore::IsDirectory(destSubDir), false);
    EXPECT_EQ(CopyCore::IsFile(destSubDir + "/sub_file.txt"), false);

    remove(subFile.c_str());
    rmdir(subDir.c_str());
    remove(destSubFile.c_str());
    rmdir(destSubDir.c_str());
    rmdir(destSrcDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopyDirFunc_001";
}

/**
 * @tc.name: CopyCoreTest_ExecLocal_001
 * @tc.desc: Test function of CopyCore::ExecLocal interface for file copy success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ExecLocal_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ExecLocal_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;
    auto callback = make_shared<FsCallbackObject>(nullptr);

    auto res = CopyCore::ExecLocal(infos, callback);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsFile(destFile));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ExecLocal_001";
}

/**
 * @tc.name: CopyCoreTest_ExecLocal_002
 * @tc.desc: Test function of CopyCore::ExecLocal interface for file copy FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ExecLocal_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ExecLocal_002";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = srcFile;
    auto callback = make_shared<FsCallbackObject>(nullptr);

    auto res = CopyCore::ExecLocal(infos, callback);
    EXPECT_EQ(res, EINVAL);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ExecLocal_002";
}

/**
 * @tc.name: CopyCoreTest_RegisterListener_001
 * @tc.desc: Test function of CopyCore::RegisterListener interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_RegisterListener_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_RegisterListener_001";

    auto infos = make_shared<FsFileInfos>();
    auto callback = CopyCore::RegisterListener(infos);
    EXPECT_NE(callback, nullptr);

    {
        std::lock_guard<std::recursive_mutex> lock(CopyCore::mutex_);
        auto iter = CopyCore::callbackMap_.find(*infos);
        EXPECT_NE(iter, CopyCore::callbackMap_.end());
    }

    CopyCore::UnregisterListener(infos);
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_RegisterListener_001";
}

/**
 * @tc.name: CopyCoreTest_UnregisterListener_001
 * @tc.desc: Test function of CopyCore::UnregisterListener interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_UnregisterListener_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_UnregisterListener_001";

    auto infos = make_shared<FsFileInfos>();
    auto callback = CopyCore::RegisterListener(infos);
    EXPECT_NE(callback, nullptr);

    CopyCore::UnregisterListener(infos);

    {
        std::lock_guard<std::recursive_mutex> lock(CopyCore::mutex_);
        auto iter = CopyCore::callbackMap_.find(*infos);
        EXPECT_EQ(iter, CopyCore::callbackMap_.end());
    }

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_UnregisterListener_001";
}

/**
 * @tc.name: CopyCoreTest_DoCopy_001
 * @tc.desc: Test function of CopyCore::DoCopy interface for failure (invalid params).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_DoCopy_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_DoCopy_001";

    string src = "invalid:/" + srcFile;
    string dest = "invalid:/" + destFile;
    optional<CopyOptions> options;

    auto res = CopyCore::DoCopy(src, dest, options);
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), E_PARAMS);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_DoCopy_001";
}

/**
 * @tc.name: CopyCoreTest_DoCopy_002
 * @tc.desc: Test function of CopyCore::DoCopy interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_DoCopy_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_DoCopy_002";

    string src = "file:///data/test/src/src.txt";
    string dest = "file:///data/test/dest/dest.txt";
    optional<CopyOptions> options;

    auto res = CopyCore::DoCopy(src, dest, options);
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_DoCopy_002";
}

/**
 * @tc.name: CopyCoreTest_GetDirSize_001
 * @tc.desc: Test function of CopyCore::GetDirSize interface for file copy success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetDirSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetDirSize_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;

    auto res = CopyCore::GetDirSize(infos, srcDir);
    EXPECT_EQ(res, 0);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetDirSize_001";
}

/**
 * @tc.name: CopyCoreTest_GetUVEntry_001
 * @tc.desc: Test function of CopyCore::GetUVEntry interface for file copy FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetUVEntry_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetUVEntry_001";

    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::GetUVEntry(infos);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetUVEntry_001";
}

/**
 * @tc.name: CopyCoreTest_CheckFileValid_001
 * @tc.desc: Test function of CopyCore::CheckFileValid interface for file copy FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CheckFileValid_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CheckFileValid_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;

    auto res = CopyCore::CheckFileValid(srcFile, infos);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CheckFileValid_001";
}

/**
 * @tc.name: CopyCoreTest_UpdateProgressSize_001
 * @tc.desc: Test function of CopyCore::UpdateProgressSize interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_UpdateProgressSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_UpdateProgressSize_001";

    auto receivedInfo = make_shared<ReceiveInfo>();
    auto callback = make_shared<FsCallbackObject>(nullptr);

    auto res = CopyCore::UpdateProgressSize(srcFile, receivedInfo, callback);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_UpdateProgressSize_001";
}

/**
 * @tc.name: CopyCoreTest_GetRegisteredListener_001
 * @tc.desc: Test function of CopyCore::GetRegisteredListener interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetRegisteredListener_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetRegisteredListener_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;

    auto res = CopyCore::GetRegisteredListener(infos);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetRegisteredListener_001";
}

/**
 * @tc.name: CopyCoreTest_SubscribeLocalListener_001
 * @tc.desc: Test function of CopyCore::SubscribeLocalListener interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_SubscribeLocalListener_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_SubscribeLocalListener_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;
    auto callback = make_shared<FsCallbackObject>(nullptr);

    auto res = CopyCore::SubscribeLocalListener(infos, callback);
    EXPECT_EQ(res, errno);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_SubscribeLocalListener_001";
}

/**
 * @tc.name: CopyCoreTest_GetRealPath_001
 * @tc.desc: Test function of CopyCore::GetRealPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetRealPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetRealPath_001";

    string path = "./data/test/src/src.txt";

    auto res = CopyCore::GetRealPath(path);
    EXPECT_EQ(res, "data/test/src/src.txt");

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetRealPath_001";
}

/**
 * @tc.name: CopyCoreTest_GetRealPath_002
 * @tc.desc: Test function of CopyCore::GetRealPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetRealPath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetRealPath_002";

    string path = "../data/test/src/src.txt";

    auto res = CopyCore::GetRealPath(path);
    EXPECT_EQ(res, "data/test/src/src.txt");

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetRealPath_002";
}

/**
 * @tc.name: CopyCoreTest_ExecCopy_001
 * @tc.desc: Test function of CopyCore::ExecCopy interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ExecCopy_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ExecCopy_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = false;
    infos->srcPath = "/data/test/src";
    infos->destPath = "/data/test/dest";

    auto res = CopyCore::ExecCopy(infos);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ExecCopy_001";
}

/**
 * @tc.name: CopyCoreTest_CopyFile_001
 * @tc.desc: Test function of CopyCore::CopyFile interface for file copy FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopyFile_001";

    string src = "datashare:///media/src_test.jpg";
    string dest = "datashare:///media/dest_test.jpg";
    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = src;
    infos->destPath = dest;

    auto res = CopyCore::CopyFile(src, dest, infos);
    EXPECT_EQ(res, errno);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopyFile_001";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_001
 * @tc.desc: Test function of CopyCore::HandleProgress interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_HandleProgress_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_HandleProgress_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = false;
    infos->srcPath = "/data/test/src";
    infos->destPath = "/data/test/dest";

    auto event = make_unique<inotify_event>();
    const int testWd = 123;
    event->wd = testWd;
    event->mask = IN_MODIFY;
    event->len = 0;
    // 执行处理
    auto [continueProcess, errCode, needSend] = CopyCore::HandleProgress(event.get(), infos, nullptr);
    
    // 验证结果
    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, EINVAL);
    EXPECT_FALSE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_001";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_002
 * @tc.desc: Test function of CopyCore::HandleProgress interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_HandleProgress_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_HandleProgress_002";

    auto infos = make_shared<FsFileInfos>();
    infos->srcPath = srcDir;
    infos->destPath = destDir;
    infos->isFile = true;

    auto callback = make_shared<FsCallbackObject>(nullptr);

    // 添加接收信息（使用不同的wd）
    const int testWd = 123;
    const int differentWd = 456;
    auto receiveInfo = make_shared<ReceiveInfo>();
    receiveInfo->path = testDir;
    callback->wds.push_back({differentWd, receiveInfo});

    // 初始化事件结构
    auto event = make_unique<inotify_event>();
    event->wd = testWd;
    event->mask = IN_MODIFY;
    event->len = 0;
    
    // 执行处理
    auto [continueProcess, errCode, needSend] = 
        CopyCore::HandleProgress(event.get(), infos, callback);
    
    // 验证结果
    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, EINVAL);
    EXPECT_FALSE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_002";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_003
 * @tc.desc: Test function of CopyCore::HandleProgress interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_HandleProgress_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_HandleProgress_003";
    
    // 创建测试用的文件信息对象
    auto infos = make_shared<FsFileInfos>();
    infos->srcPath = srcDir;
    infos->destPath = destDir;
    infos->isFile = true;
    
    // 创建测试用的回调对象
    auto callback = make_shared<FsCallbackObject>(nullptr);
    
    // 添加匹配的接收信息
    const int testWd = 123;
    auto receiveInfo = make_shared<ReceiveInfo>();
    receiveInfo->path = srcFile;
    callback->wds.push_back({testWd, receiveInfo});
    
    // 初始化事件结构
    auto event = make_unique<inotify_event>();
    event->wd = testWd;
    event->mask = IN_MODIFY;
    event->len = 0;
    
    // 执行处理
    auto [continueProcess, errCode, needSend] = CopyCore::HandleProgress(event.get(), infos, callback);
    
    // 验证结果
    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, ERRNO_NOERR);
    EXPECT_TRUE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test