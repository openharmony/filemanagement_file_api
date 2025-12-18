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
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CopyCoreTest";
    const string srcDir = testDir + "/srcDir";
    const string destDir = testDir + "/destDir";
    const string srcFile = srcDir + "/src.txt";
    const string destFile = destDir + "/dest.txt";
};

void CopyCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CopyCoreMockTest");
}

void CopyCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0;
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(srcDir));
    ASSERT_TRUE(FileUtils::CreateDirectories(destDir));
    ASSERT_TRUE(FileUtils::CreateFile(srcFile));
}

void CopyCoreTest::TearDown()
{
    CopyCore::callbackMap_.clear();
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t EXPECTED_WD = 100;
inline const int32_t UNEXPECTED_WD = 200;

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

    string validUri = "file://" + srcDir + "/CopyCoreTest_IsValidUri_001.txt";
    auto res = CopyCore::IsValidUri(validUri);
    EXPECT_TRUE(res);

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

    string invalidUri = "invalid:/" + srcDir + "/CopyCoreTest_IsValidUri_002.txt";
    auto res = CopyCore::IsValidUri(invalidUri);
    EXPECT_FALSE(res);

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

    string remoteUri = "file://" + srcDir + "/CopyCoreTest_IsRemoteUri_001.txt?networkid=123";
    auto res = CopyCore::IsRemoteUri(remoteUri);
    EXPECT_TRUE(res);

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

    string localUri = "file://" + srcDir + "/CopyCoreTest_IsRemoteUri_002.txt";
    auto res = CopyCore::IsRemoteUri(localUri);
    EXPECT_FALSE(res);

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
    EXPECT_TRUE(res);

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
    EXPECT_FALSE(res);

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
    EXPECT_TRUE(res);

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
    EXPECT_FALSE(res);

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
    EXPECT_FALSE(res);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_IsMediaUri_001";
}

/**
 * @tc.name: CopyCoreTest_GetFileSize_001
 * @tc.desc: Test function of CopyCore::GetFileSize interface for SUCCESS.
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
 * @tc.desc: Test function of CopyCore::GetFileSize interface for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_GetFileSize_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_GetFileSize_002";

    string nonExistentFile = srcDir + "/non_existent.txt";
    auto [err, size] = CopyCore::GetFileSize(nonExistentFile);
    EXPECT_NE(err, ERRNO_NOERR);
    EXPECT_EQ(size, 0);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetFileSize_002";
}

/**
 * @tc.name: CopyCoreTest_CheckOrCreatePath_001
 * @tc.desc: Test function of CopyCore::CheckOrCreatePath interface for SUCCESS.
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
 * @tc.desc: Test function of CopyCore::CheckOrCreatePath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CheckOrCreatePath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CheckOrCreatePath_002";

    string newFile = destDir + "/CopyCoreTest_CheckOrCreatePath_002.txt";
    auto res = CopyCore::CheckOrCreatePath(newFile);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsFile(newFile));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CheckOrCreatePath_002";
}

/**
 * @tc.name: CopyCoreTest_MakeDir_001
 * @tc.desc: Test function of CopyCore::MakeDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_MakeDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_MakeDir_001";

    string newDir = destDir + "/CopyCoreTest_MakeDir_001";
    auto res = CopyCore::MakeDir(newDir);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(newDir));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_MakeDir_001";
}

/**
 * @tc.name: CopyCoreTest_MakeDir_002
 * @tc.desc: Test function of CopyCore::MakeDir interface for SUCCESS when path is already exits.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
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
 * @tc.desc: Test function of CopyCore::MakeDir interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_MakeDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_MakeDir_003";

    auto path = srcDir + "/non_existent_dir/non_existent_subdir";
    auto res = CopyCore::MakeDir(path);
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

    string srcFile = "invalid:/" + srcDir + "/CopyCoreTest_ValidParams_001.txt";

    auto res = CopyCore::ValidParams(srcFile, destFile);
    EXPECT_FALSE(res);

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

    string destFile = "invalid:/" + destDir + "/CopyCoreTest_ValidParams_002.txt";

    auto res = CopyCore::ValidParams(srcFile, destFile);
    EXPECT_FALSE(res);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_002";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_003
 * @tc.desc: Test function of CopyCore::ValidParams interface for TRUE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_003";

    string src = "file://" + srcFile;
    string dest = "file://" + destFile;

    auto res = CopyCore::ValidParams(src, dest);
    EXPECT_TRUE(res);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_003";
}

/**
 * @tc.name: CopyCoreTest_CreateFileInfos_001
 * @tc.desc: Test function of CopyCore::CreateFileInfos interface for SUCCESS.
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
    EXPECT_NE(infos, nullptr);
    if (infos) {
        EXPECT_FALSE(infos->hasListener);
        EXPECT_EQ(infos->listener, nullptr);
        EXPECT_EQ(infos->taskSignal, nullptr);
    }

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CreateFileInfos_001";
}

/**
 * @tc.name: CopyCoreTest_CopySubDir_001
 * @tc.desc: Test function of CopyCore::CopySubDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopySubDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopySubDir_001";

    string subDir = srcDir + "/CopyCoreTest_CopySubDir_001";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreTest_CopySubDir_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    string destSubDir = destDir + "/CopyCoreTest_CopySubDir_001";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    string destSubFile = destSubDir + "/CopyCoreTest_CopySubDir_001.txt";
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(destSubDir));
    EXPECT_TRUE(CopyCore::IsFile(destSubFile));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopySubDir_001";
}

/**
 * @tc.name: CopyCoreTest_CopySubDir_002
 * @tc.desc: Test function of CopyCore::CopySubDir interface for SUCCESS without notify.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopySubDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopySubDir_002";

    string subDir = srcDir + "/CopyCoreTest_CopySubDir_002";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreTest_CopySubDir_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    string destSubDir = destDir + "/CopyCoreTest_CopySubDir_002";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopySubDir_002";
}

/**
 * @tc.name: CopyCoreTest_RecurCopyDir_001
 * @tc.desc: Test function of CopyCore::RecurCopyDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_RecurCopyDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_RecurCopyDir_001";

    string subDir = srcDir + "/CopyCoreTest_RecurCopyDir_001";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreTest_RecurCopyDir_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    string destSubDir = destDir + "/CopyCoreTest_RecurCopyDir_001";
    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::RecurCopyDir(srcDir, destDir, infos);
    string destSubFile = destSubDir + "/CopyCoreTest_RecurCopyDir_001.txt";
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(destSubDir));
    EXPECT_TRUE(CopyCore::IsFile(destSubDir + "/CopyCoreTest_RecurCopyDir_001.txt"));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_RecurCopyDir_001";
}

/**
 * @tc.name: CopyCoreTest_CopyDirFunc_001
 * @tc.desc: Test function of CopyCore::CopyDirFunc interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopyDirFunc_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopyDirFunc_001";

    string copiedDir = destDir + "/CopyCoreTest";
    string copiedFile = copiedDir + "/src.txt";

    auto infos = make_shared<FsFileInfos>();
    auto res = CopyCore::CopyDirFunc(srcDir, destDir, infos);
    EXPECT_EQ(res, ERRNO_NOERR);
    EXPECT_TRUE(CopyCore::IsDirectory(copiedDir));
    EXPECT_TRUE(CopyCore::IsFile(copiedFile));

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopyDirFunc_001";
}

/**
 * @tc.name: CopyCoreTest_ExecLocal_001
 * @tc.desc: Test function of CopyCore::ExecLocal interface for SUCCESS.
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
 * @tc.desc: Test function of CopyCore::ExecLocal interface for FAILURE.
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
 * @tc.desc: Test function of CopyCore::RegisterListener interface for SUCCESS.
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
 * @tc.name: CopyCoreTest_RegisterListener_002
 * @tc.desc: Test function of CopyCore::RegisterListener interface for the case when the info already exists in
 * callbackMap_.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_RegisterListener_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_RegisterListener_002";

    auto infos = make_shared<FsFileInfos>();
    auto firstCallback = CopyCore::RegisterListener(infos);
    EXPECT_NE(firstCallback, nullptr);
    auto secondCallback = CopyCore::RegisterListener(infos);
    EXPECT_EQ(secondCallback, nullptr);

    {
        std::lock_guard<std::recursive_mutex> lock(CopyCore::mutex_);
        auto iter = CopyCore::callbackMap_.find(*infos);
        EXPECT_NE(iter, CopyCore::callbackMap_.end());
    }

    CopyCore::UnregisterListener(infos);
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_RegisterListener_002";
}

/**
 * @tc.name: CopyCoreTest_UnregisterListener_001
 * @tc.desc: Test function of CopyCore::UnregisterListener interface for SUCCESS.
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
 * @tc.name: CopyCoreTest_UnregisterListener_002
 * @tc.desc: Test function of CopyCore::UnregisterListener interface for the case when the info is not registered in
 * callbackMap_.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_UnregisterListener_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_UnregisterListener_002";

    auto infos = make_shared<FsFileInfos>();

    CopyCore::UnregisterListener(infos);

    {
        std::lock_guard<std::recursive_mutex> lock(CopyCore::mutex_);
        auto iter = CopyCore::callbackMap_.find(*infos);
        EXPECT_EQ(iter, CopyCore::callbackMap_.end());
    }

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_UnregisterListener_002";
}

/**
 * @tc.name: CopyCoreTest_DoCopy_001
 * @tc.desc: Test function of CopyCore::DoCopy interface for FAILURE.
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
    EXPECT_EQ(err.GetErrNo(), E_PARAMS); // errno is 401
    EXPECT_EQ(err.GetErrMsg(), "The input parameter is invalid");

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_DoCopy_001";
}

/**
 * @tc.name: CopyCoreTest_DoCopy_002
 * @tc.desc: Test function of CopyCore::DoCopy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_DoCopy_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_DoCopy_002";

    string src = "file://" + srcFile;
    string dest = "file://" + destFile;
    optional<CopyOptions> options;

    auto res = CopyCore::DoCopy(src, dest, options);
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_DoCopy_002";
}

/**
 * @tc.name: CopyCoreTest_GetDirSize_001
 * @tc.desc: Test function of CopyCore::GetDirSize interface for SUCCESS.
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
 * @tc.desc: Test function of CopyCore::GetUVEntry interface for FAILURE.
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
 * @tc.desc: Test function of CopyCore::CheckFileValid interface for FALSE.
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
    EXPECT_FALSE(res);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CheckFileValid_001";
}

/**
 * @tc.name: CopyCoreTest_UpdateProgressSize_001
 * @tc.desc: Test function of CopyCore::UpdateProgressSize interface for SUCCESS.
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
 * @tc.desc: Test function of CopyCore::GetRegisteredListener interface for FAILURE.
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
 * @tc.desc: Test function of CopyCore::SubscribeLocalListener interface for FAILURE.
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
    EXPECT_NE(res, ERRNO_NOERR);
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

    string path = "./data/unittest/CopyCoreTest/CopyCoreTest_GetRealPath_001";

    auto res = CopyCore::GetRealPath(path);
    EXPECT_EQ(res, "data/unittest/CopyCoreTest/CopyCoreTest_GetRealPath_001");

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

    string path = "/data/unittest/CopyCoreTest/../CopyCoreTest_GetRealPath_002";

    auto res = CopyCore::GetRealPath(path);
    EXPECT_EQ(res, "/data/unittest/CopyCoreTest_GetRealPath_002");

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_GetRealPath_002";
}

/**
 * @tc.name: CopyCoreTest_ExecCopy_001
 * @tc.desc: Test function of CopyCore::ExecCopy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ExecCopy_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ExecCopy_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = false;
    infos->srcPath = srcDir;
    infos->destPath = destDir;

    auto res = CopyCore::ExecCopy(infos);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ExecCopy_001";
}

/**
 * @tc.name: CopyCoreTest_CopyFile_001
 * @tc.desc: Test function of CopyCore::CopyFile interface for FAILURE when coping datashare file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CopyFile_001";

    string src = "datashare:///media/CopyCoreTest_CopyFile_001/src.jpg";
    string dest = "datashare:///media/CopyCoreTest_CopyFile_001/dest.jpg";
    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = src;
    infos->destPath = dest;

    auto res = CopyCore::CopyFile(src, dest, infos);
    EXPECT_EQ(res, ENOENT);
    EXPECT_EQ(res, errno);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CopyFile_001";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_001
 * @tc.desc: Test function of CopyCore::HandleProgress interface for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_HandleProgress_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_HandleProgress_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = false;
    infos->srcPath = srcFile;
    infos->destPath = destFile;

    auto event = make_unique<inotify_event>();
    event->wd = EXPECTED_WD;
    event->mask = IN_MODIFY;
    event->len = 0;
    auto [continueProcess, errCode, needSend] = CopyCore::HandleProgress(event.get(), infos, nullptr);

    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, EINVAL);
    EXPECT_FALSE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_001";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_002
 * @tc.desc: Test function of CopyCore::HandleProgress interface for FAILURE.
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

    auto receiveInfo = make_shared<ReceiveInfo>();
    receiveInfo->path = testDir;
    callback->wds.push_back({ UNEXPECTED_WD, receiveInfo });

    auto event = make_unique<inotify_event>();
    event->wd = EXPECTED_WD;
    event->mask = IN_MODIFY;
    event->len = 0;

    auto [continueProcess, errCode, needSend] = CopyCore::HandleProgress(event.get(), infos, callback);

    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, EINVAL);
    EXPECT_FALSE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_002";
}

/**
 * @tc.name: CopyCoreTest_HandleProgress_003
 * @tc.desc: Test function of CopyCore::HandleProgress interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_HandleProgress_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_HandleProgress_003";

    auto infos = make_shared<FsFileInfos>();
    infos->srcPath = srcDir;
    infos->destPath = destDir;
    infos->isFile = true;

    auto callback = make_shared<FsCallbackObject>(nullptr);

    auto receiveInfo = make_shared<ReceiveInfo>();
    receiveInfo->path = srcFile;
    callback->wds.push_back({ EXPECTED_WD, receiveInfo });

    auto event = make_unique<inotify_event>();
    event->wd = EXPECTED_WD;
    event->mask = IN_MODIFY;
    event->len = 0;

    auto [continueProcess, errCode, needSend] = CopyCore::HandleProgress(event.get(), infos, callback);

    EXPECT_TRUE(continueProcess);
    EXPECT_EQ(errCode, ERRNO_NOERR);
    EXPECT_TRUE(needSend);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_HandleProgress_003";
}

/**
 * @tc.name: CopyCoreTest_OnFileReceive_001
 * @tc.desc: Test function of CopyCore::OnFileReceive interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_OnFileReceive_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_OnFileReceive_001";

    auto infos = make_shared<FsFileInfos>();
    CopyCore::OnFileReceive(infos);
    EXPECT_TRUE(CopyCore::callbackMap_.empty());

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_OnFileReceive_001";
}

/**
 * @tc.name: CopyCoreTest_OnFileReceive_002
 * @tc.desc: Test function of CopyCore::OnFileReceive interface when listener is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_OnFileReceive_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_OnFileReceive_002";

    auto infos = make_shared<FsFileInfos>();
    auto callback = CopyCore::RegisterListener(infos);
    CopyCore::OnFileReceive(infos);
    EXPECT_NE(callback, nullptr);
    if (callback) {
        EXPECT_EQ(callback->listener, nullptr);
    }
    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_OnFileReceive_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
