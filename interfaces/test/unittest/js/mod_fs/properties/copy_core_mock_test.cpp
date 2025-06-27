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
#include "unistd_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreMockTest : public testing::Test {
public:
    static constexpr mode_t permission0755 = 0755;
    static constexpr mode_t permission0644 = 0644;
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
    mkdir(testDir.c_str(), permission0755);
    mkdir(srcDir.c_str(), permission0755);
    mkdir(destDir.c_str(), permission0755);
    int32_t fd = open(srcFile.c_str(), O_CREAT | O_RDWR, permission0644);
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
    UnistdMock::DestroyMock();
    InotifyMock::DestroyMock();
}

void CopyCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
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
    auto unistdMock = UnistdMock::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
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
    auto unistdMock = UnistdMock::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto res = CopyCore::DoCopy(srcUri, destUri, options);
    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_TRUE(filesystem::exists(destFile));
    int ret = remove(destFile.c_str());
    EXPECT_TRUE(ret == 0);

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
    mkdir(subDir.c_str(), permission0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, permission0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);

    auto inotifyMock = InotifyMock::GetMock();
    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto unistdMock = UnistdMock::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    EXPECT_EQ(res, UNKNOWN_ERR);

    int ret = remove(subFile.c_str());
    EXPECT_TRUE(ret == 0);
    rmdir(subDir.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
