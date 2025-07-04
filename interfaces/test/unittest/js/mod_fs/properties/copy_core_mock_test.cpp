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
#include "mock_progress_listener.h"
#include "mock/uv_fs_mock.h"
#include "poll_mock.h"
#include "unistd_mock.h"

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

private:
    static constexpr mode_t permission0755 = 0755;
    static constexpr mode_t permission0644 = 0644;
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
    InotifyMock::EnableMock();
    PollMock::EnableMock();
    UnistdMock::EnableMock();
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
    InotifyMock::DisableMock();
    PollMock::DisableMock();
    UnistdMock::DisableMock();
}

void CopyCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyCoreMockTest::TearDown(void)
{
    CopyCore::callbackMap_.clear();
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

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_002
 * @tc.desc: Test CopyCore::CopySubDir when iter == CopyCore::callbackMap_.end()
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_002";

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
    CopyCore::callbackMap_.clear();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    EXPECT_EQ(res, UNKNOWN_ERR);

    int ret = remove(subFile.c_str());
    EXPECT_TRUE(ret == 0);
    rmdir(subDir.c_str());
    rmdir(destSubDir.c_str());
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_002";
}

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_003
 * @tc.desc: Test CopyCore::CopySubDir when iter->second == nullptr
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_003";

    string subDir = srcDir + "/sub_dir";
    mkdir(subDir.c_str(), permission0755);
    string subFile = subDir + "/sub_file.txt";
    int fd = open(subFile.c_str(), O_CREAT | O_RDWR, permission0644);
    if (fd < 0) {
        GTEST_LOG_(INFO) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        EXPECT_TRUE(false);
    }
    close(fd);

    auto inotifyMock = InotifyMock::GetMock();
    string destSubDir = destDir + "/sub_dir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto unistdMock = UnistdMock::GetMock();
    CopyCore::callbackMap_[*infos] = nullptr;

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);
    EXPECT_EQ(res, UNKNOWN_ERR);

    int ret = remove(subFile.c_str());
    EXPECT_TRUE(ret == 0);
    rmdir(subDir.c_str());
    rmdir(destSubDir.c_str());
    CopyCore::callbackMap_.clear();

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_003";
}

/**
 * @tc.name: CopyCoreMockTest_ReceiveComplete_001
 * @tc.desc: Test CopyCore::ReceiveComplete in normal case
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReceiveComplete_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReceiveComplete_001";

    auto mockListener = std::make_shared<MockProgressListener>();
    auto callback = std::make_shared<FsCallbackObject>(mockListener);
    callback->maxProgressSize = 50;
    auto entry = std::make_shared<FsUvEntry>(callback);
    entry->progressSize = 100;
    entry->totalSize = 200;

    EXPECT_CALL(*mockListener, InvokeListener(entry->progressSize, entry->totalSize)).Times(1);
    CopyCore::ReceiveComplete(entry);

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReceiveComplete_001";
}

/**
 * @tc.name: CopyCoreMockTest_ReceiveComplete_002
 * @tc.desc: Test CopyCore::ReceiveComplete when processedSize < entry->callback->maxProgressSize
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReceiveComplete_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReceiveComplete_002";

    auto mockListener = std::make_shared<MockProgressListener>();
    auto callback = std::make_shared<FsCallbackObject>(mockListener);
    callback->maxProgressSize = 100;
    auto entry = std::make_shared<FsUvEntry>(callback);
    entry->progressSize = 50; // Mock valid progressSize
    entry->totalSize = 200;   // Mock valid totalSize, and progressSize < totalSize

    EXPECT_CALL(*mockListener, InvokeListener(testing::_, testing::_)).Times(0);
    CopyCore::ReceiveComplete(entry);

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReceiveComplete_002";
}

/**
 * @tc.name: CopyCoreMockTest_ReadNotifyEvent_001
 * @tc.desc: Test function of CopyCoreMockTest::ReadNotifyEvent interface for SUCCESS when read valid event data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReadNotifyEvent_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReadNotifyEvent_001";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    infos->run = true;
    auto callback = CopyCore::RegisterListener(infos);
    int32_t len = static_cast<int32_t>(sizeof(struct inotify_event));
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(len));
    // Do testing
    CopyCore::ReadNotifyEvent(infos);
    // Verify results
    EXPECT_NE(callback, nullptr);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReadNotifyEvent_001";
}

/**
 * @tc.name: CopyCoreMockTest_ReadNotifyEvent_002
 * @tc.desc: Test function of CopyCore::ReadNotifyEvent interface for FAILURE when read returns -1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReadNotifyEvent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReadNotifyEvent_002";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    infos->run = true;
    auto callback = CopyCore::RegisterListener(infos);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    CopyCore::ReadNotifyEvent(infos);
    // Verify results
    EXPECT_NE(callback, nullptr);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_EQ(errno, EIO);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReadNotifyEvent_002";
}

/**
 * @tc.name: CopyCoreMockTest_ReadNotifyEvent_003
 * @tc.desc: Test function of CopyCore::ReadNotifyEvent interface for SUCCESS when read returns 0 (EOF).
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReadNotifyEvent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReadNotifyEvent_003";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    infos->run = true;
    auto callback = CopyCore::RegisterListener(infos);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(0, 0));
    // Do testing
    CopyCore::ReadNotifyEvent(infos);
    // Verify results
    EXPECT_NE(callback, nullptr);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_EQ(errno, 0);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReadNotifyEvent_003";
}

/**
 * @tc.name: CopyCoreMockTest_ReadNotifyEventLocked_001
 * @tc.desc: Test ReadNotifyEventLocked when closed is false.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReadNotifyEventLocked_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReadNotifyEventLocked_001";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    auto callback = std::make_shared<FsCallbackObject>(nullptr);
    callback->closed = false;
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    // Do testing
    CopyCore::ReadNotifyEventLocked(infos, callback);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(callback->reading);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReadNotifyEventLocked_001";
}

/**
 * @tc.name: CopyCoreMockTest_ReadNotifyEventLocked_002
 * @tc.desc: Test ReadNotifyEventLocked when close after read.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_ReadNotifyEventLocked_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_ReadNotifyEventLocked_002";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    auto callback = std::make_shared<FsCallbackObject>(nullptr);
    callback->closed = false;
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([callback](int fd, void *buf, size_t count) {
            errno = EIO;
            callback->closed = true;
            return 0;
        });
    EXPECT_CALL(*unistdMock, close(testing::_)).WillRepeatedly(testing::Return(0));
    // Do testing
    CopyCore::ReadNotifyEventLocked(infos, callback);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(callback->closed);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_ReadNotifyEventLocked_002";
}

/**
 * @tc.name: CopyCoreMockTest_GetNotifyId_001
 * @tc.desc: Test function of CopyCore::GetNotifyId interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_GetNotifyId_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_GetNotifyId_001";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    infos->run = true;
    infos->exceptionCode = ERRNO_NOERR;
    infos->eventFd = 1;
    infos->notifyFd = 1;

    auto callback = CopyCore::RegisterListener(infos);
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([infos](struct pollfd *fds, nfds_t n, int timeout) {
            fds[1].revents = POLLIN;
            infos->run = false; // Ensure the loop will exit
            return 1;
        });
    // Do testing
    CopyCore::GetNotifyEvent(infos);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_NE(callback, nullptr);
    EXPECT_FALSE(infos->run);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_GetNotifyId_001";
}

/**
 * @tc.name: CopyCoreMockTest_GetNotifyId_002
 * @tc.desc: Test function of CopyCore::GetNotifyId interface fails when callback is nullptr.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_GetNotifyId_002, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_GetNotifyId_002";
    // Prepare test condition
    auto infos = make_shared<FsFileInfos>();
    // Do testing
    CopyCore::GetNotifyEvent(infos);
    // Verify results
    EXPECT_EQ(infos->exceptionCode, EINVAL);
    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_GetNotifyId_002";
}

/**
 * @tc.name: CopyCoreMockTest_CreateFileInfos_001
 * @tc.desc: Test function of CopyCore::CreateFileInfos interface with listener and copySignal.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CreateFileInfos_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CreateFileInfos_001";

    auto copySignal = std::make_shared<FsTaskSignal>();
    copySignal->taskSignal_ = std::make_shared<TaskSignal>();
    auto options = std::make_optional<CopyOptions>();
    options->progressListener = std::make_shared<MockProgressListener>();
    options->copySignal = std::move(copySignal);

    auto [errCode, infos] = CopyCore::CreateFileInfos(srcFile, destFile, options);
    EXPECT_EQ(errCode, ERRNO_NOERR);
    EXPECT_NE(infos, nullptr);
    if (infos) {
        EXPECT_TRUE(infos->hasListener);
        EXPECT_NE(infos->listener, nullptr);
        EXPECT_NE(infos->taskSignal, nullptr);
    }

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CreateFileInfos_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
