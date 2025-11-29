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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "inotify_mock.h"
#include "mock_progress_listener.h"
#include "poll_mock.h"
#include "unistd_mock.h"
#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/CopyCoreMockTest";
    const string srcDir = testDir + "/srcDir";
    const string destDir = testDir + "/destDir";
    const string srcFile = srcDir + "/src.txt";
    const string destFile = destDir + "/dest.txt";
};

void CopyCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CopyCoreMockTest");
    UvFsMock::EnableMock();
    InotifyMock::EnableMock();
    PollMock::EnableMock();
    UnistdMock::EnableMock();
}

void CopyCoreMockTest::TearDownTestCase()
{
    UvFsMock::DisableMock();
    InotifyMock::DisableMock();
    PollMock::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0;
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(srcDir));
    ASSERT_TRUE(FileUtils::CreateDirectories(destDir));
    ASSERT_TRUE(FileUtils::CreateFile(srcFile));
}

void CopyCoreMockTest::TearDown()
{
    CopyCore::callbackMap_.clear();
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyCoreMockTest_CopyFile_001
 * @tc.desc: Test function of CopyCore::CopyFile interface for FAILURE when uv_fs_sendfile fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopyFile_001";

    auto infos = make_shared<FsFileInfos>();
    infos->isFile = true;
    infos->srcPath = srcFile;
    infos->destPath = destFile;

    auto unistdMock = UnistdMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).WillOnce(SetErrnoAndReturn(EIO, -1));

    auto res = CopyCore::CopyFile(srcFile, destFile, infos);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, errno);
    EXPECT_EQ(res, EIO);

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
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto res = CopyCore::DoCopy(srcUri, destUri, options);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());

    EXPECT_EQ(res.IsSuccess(), true);
    EXPECT_TRUE(FileUtils::Exists(destFile));

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_DoCopy_001";
}

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_001
 * @tc.desc: Test function of CopyCore::CopySubDir interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_001";

    string subDir = srcDir + "/subDir";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreMockTest_CopySubDir_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    auto inotifyMock = InotifyMock::GetMock();
    string destSubDir = destDir + "/subDir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto unistdMock = UnistdMock::GetMock();
    CopyCore::callbackMap_[*infos] = make_shared<FsCallbackObject>(nullptr);

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_001";
}

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_002
 * @tc.desc: Test function of CopyCore::CopySubDir interface for FAILURE when failed to find infos
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_002";

    string subDir = srcDir + "/subDir";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreMockTest_CopySubDir_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    auto inotifyMock = InotifyMock::GetMock();
    string destSubDir = destDir + "/subDir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto unistdMock = UnistdMock::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(res, UNKNOWN_ERR);

    GTEST_LOG_(INFO) << "CopyCoreMockTest-end CopyCoreMockTest_CopySubDir_002";
}

/**
 * @tc.name: CopyCoreMockTest_CopySubDir_003
 * @tc.desc: Test function of CopyCore::CopySubDir interface for FAILURE infos callback is nullptr
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreMockTest, CopyCoreMockTest_CopySubDir_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreMockTest-begin CopyCoreMockTest_CopySubDir_003";

    string subDir = srcDir + "/subDir";
    ASSERT_TRUE(FileUtils::CreateDirectories(subDir));
    string subFile = subDir + "/CopyCoreMockTest_CopySubDir_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(subFile));

    auto inotifyMock = InotifyMock::GetMock();
    string destSubDir = destDir + "/subDir";
    auto infos = make_shared<FsFileInfos>();
    infos->notifyFd = 1;
    auto unistdMock = UnistdMock::GetMock();
    CopyCore::callbackMap_[*infos] = nullptr;

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    auto res = CopyCore::CopySubDir(subDir, destSubDir, infos);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(res, UNKNOWN_ERR);

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
    // 当前已经拷贝的字节数
    entry->progressSize = 100;
    // 需要拷贝的字节数
    entry->totalSize = 200;

    EXPECT_CALL(*mockListener, InvokeListener(entry->progressSize, entry->totalSize)).Times(1);
    CopyCore::ReceiveComplete(entry);

    testing::Mock::VerifyAndClearExpectations(mockListener.get());

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

    testing::Mock::VerifyAndClearExpectations(mockListener.get());

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
            // Ensure the loop will exit
            infos->run = false;
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
    options->copySignal = copySignal.get();

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
