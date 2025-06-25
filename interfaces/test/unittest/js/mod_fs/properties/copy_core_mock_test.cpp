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

#include <fcntl.h>
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <gtest/gtest.h>

#include "copy_core.h"
#include "inotify_mock.h"
#include "mock/uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;

    static const string testDir;
    static const string srcDir;
    static const string destDir;
    static const string srcFile;
    static const string destFile;
};

const string CopyCoreMockTest::testDir = "/data/test";
const string CopyCoreMockTest::srcDir = testDir + "/src";
const string CopyCoreMockTest::destDir = testDir + "/dest";
const string CopyCoreMockTest::srcFile = srcDir + "/src.txt";
const string CopyCoreMockTest::destFile = destDir + "/dest.txt";

void CopyCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mkdir(testDir.c_str(), 0755);
    mkdir(srcDir.c_str(), 0755);
    mkdir(destDir.c_str(), 0755);
    int32_t fd = open(srcFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void CopyCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    int ret = remove(srcFile.c_str());
    EXPECT_TRUE(ret == 0);
    rmdir(srcDir.c_str());
    rmdir(destDir.c_str());
    rmdir(testDir.c_str());
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void CopyCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    remove(destFile.c_str());
}

/**
 * @tc.name: CopyCoreMockTest_CopyFile_001
 * @tc.desc: Test function of CopyCore::CopyFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopyFile_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = CopyCoreMockTest::srcFile;
    infos->destPath = CopyCoreMockTest::destFile;

    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = CopyCore::CopyFile(srcFile, destFile, infos);
    EXPECT_EQ(res, errno);

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopyFile_001";
}

/**
 * @tc.name: CopyCoreMockTest_DoCopy_001
 * @tc.desc: Test function of CopyCore::DoCopy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_DoCopy_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_DoCopy_001";

    string srcUri = "file://" + srcFile;
    string destUri = "file://" + destFile;
    optional<CopyOptions> options;

    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto res = CopyCore::DoCopy(srcUri, destUri, options);
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_TRUE(filesystem::exists(destFile));

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_DoCopy_001";
}

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_001
 * @tc.desc: Test function of CopyCore::CopySubDir interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_001";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), 0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);

    testing::StrictMock<InotifyMock> &inotifyMock = static_cast<testing::StrictMock<InotifyMock> &>(GetInotifyMock());
    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    EXPECT_CALL(inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    testing::Mock::VerifyAndClearExpectations(&inotifyMock);
    EXPECT_EQ(res, UNKNOWN_ERR);

    remove(subFile.c_str());
    rmdir(subDir.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
