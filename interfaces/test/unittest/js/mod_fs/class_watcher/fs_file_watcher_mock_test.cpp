/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "eventfd_mock.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"
#include "inotify_mock.h"
#include "mock_watcher_callback.h"
#include "poll_mock.h"
#include "securec.h"
#include "unistd_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class FsFileWatcherMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FsFileWatcherMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsFileWatcherMockTest::TearDownTestCase(void)
{
    EventfdMock::DestroyMock();
    InotifyMock::DestroyMock();
    PollMock::DestroyMock();
    UnistdMock::DestroyMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsFileWatcherMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void FsFileWatcherMockTest::TearDown(void)
{
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.taskRunning_ = false;
    watcher.run_ = false;
    watcher.reading_ = false;
    watcher.closed_ = false;
    watcher.notifyFd_ = -1;
    watcher.eventFd_ = -1;
    watcher.dataCache_.ClearCache();
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyId_001
 * @tc.desc: Test function of FsFileWatcher::GetNotifyId interface.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyId_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyId_001";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    int32_t expected = -1;
    // Do testing
    int32_t result = watcher.GetNotifyId();
    // Verify results
    EXPECT_EQ(result, expected);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyId_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_InitNotify_001
 * @tc.desc: Test function of FsFileWatcher::InitNotify interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_InitNotify_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_InitNotify_001";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Set mock behaviors
    auto eventfdMock = EventfdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::Return(1));
    EXPECT_CALL(*eventfdMock, eventfd(testing::_, testing::_)).Times(1).WillOnce(testing::Return(2));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(eventfdMock.get());
    EXPECT_TRUE(result);
    EXPECT_EQ(watcher.notifyFd_, 1);
    EXPECT_EQ(watcher.eventFd_, 2);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_InitNotify_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_InitNotify_002
 * @tc.desc: Test function of FsFileWatcher::InitNotify interface for FAILURE when inotify_init fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_InitNotify_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_InitNotify_002";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_FALSE(result);
    EXPECT_EQ(watcher.notifyFd_, -1);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_InitNotify_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_InitNotify_003
 * @tc.desc: Test function of FsFileWatcher::InitNotify interface for FAILURE when eventfd fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_InitNotify_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_InitNotify_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    auto eventfdMock = EventfdMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::Return(1));
    EXPECT_CALL(*eventfdMock, eventfd(testing::_, testing::_)).Times(1).WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(eventfdMock.get());
    EXPECT_FALSE(result);
    EXPECT_EQ(watcher.notifyFd_, 1);
    EXPECT_EQ(watcher.eventFd_, -1);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_InitNotify_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_StartNotify_001
 * @tc.desc: Test function of FsFileWatcher::StartNotify interface for SUCCESS when path is not watched.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StartNotify_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StartNotify_001";
    // Prepare test parameters
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StartNotify_001";
    info->events = IN_CREATE | IN_DELETE;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    // Set mock behaviors
    int32_t expectedWd = 100;
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(expectedWd));
    // Do testing
    int32_t result = watcher.StartNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);
    EXPECT_EQ(info->wd, expectedWd);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StartNotify_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_StartNotify_002
 * @tc.desc: Test function of FsFileWatcher::StartNotify interface for SUCCESS when path is already watched with same
 * events.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StartNotify_002, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StartNotify_002";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StartNotify_002";
    info->events = IN_CREATE | IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_)).Times(0);
    // Do testing
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    int32_t result = watcher.StartNotify(info);
    // Verify results
    EXPECT_EQ(result, ERRNO_NOERR);
    EXPECT_EQ(info->wd, expectedWd);
}

/**
 * @tc.name: FsFileWatcherMockTest_StartNotify_003
 * @tc.desc: Test function of FsFileWatcher::StartNotify interface for FAILURE when info is nullptr.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StartNotify_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StartNotify_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Do testing with nullptr parameter
    int32_t result = watcher.StartNotify(nullptr);
    // Verify results
    EXPECT_EQ(result, EINVAL);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StartNotify_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_StartNotify_004
 * @tc.desc: Test function of FsFileWatcher::StartNotify interface for FAILURE when notifyFd_ is invalid.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StartNotify_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StartNotify_004";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = -1; // Invalid notifyFd
    // Build test parameters
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StartNotify_004";
    info->events = IN_CREATE;
    // Do testing
    int32_t result = watcher.StartNotify(info);
    // Verify results
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StartNotify_004";
}

/**
 * @tc.name: FsFileWatcherMockTest_StartNotify_005
 * @tc.desc: Test function of FsFileWatcher::StartNotify interface for FAILURE when inotify_add_watch fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StartNotify_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StartNotify_005";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    // Build test parameters
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StartNotify_005";
    info->events = IN_DELETE;
    // Set mock behaviors for inotify_add_watch failure
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    int32_t result = watcher.StartNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StartNotify_005";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_001
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_001";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_001";
    info->events = IN_CREATE | IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(2).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_002
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when info is nullptr.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_002";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Do testing
    int32_t result = watcher.StopNotify(nullptr);
    // Verify results
    EXPECT_EQ(result, EINVAL);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_003
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when notifyFd_ is invalid.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = -1; // Invalid notifyFd
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_003";
    info->events = IN_CREATE;
    info->wd = expectedWd;
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_004
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when inotify_rm_watch fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_004";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_004";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(2).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_004";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_005
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for SUCCESS when rm watch fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_005";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_005";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set rm watch fail condition
    watcher.closed_ = true;
    watcher.reading_ = true;
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::Return(1));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(0);
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_005";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_006
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for SUCCESS when having remainingEvents but access
 * file failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_006";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_006";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set having remainingEvents condition
    auto remainingInfo = std::make_shared<WatcherInfo>(nullptr);
    remainingInfo->fileName = "/test/FsFileWatcherMockTest_StopNotify_006";
    remainingInfo->events = IN_CREATE;
    remainingInfo->wd = expectedWd;
    watcher.dataCache_.AddWatcherInfo(remainingInfo);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(0);
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_006";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_007
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for SUCCESS when having remainingEvents and
 * NotifyToWatchNewEvents success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_007";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_007";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set having remainingEvents condition
    auto remainingInfo = std::make_shared<WatcherInfo>(nullptr);
    remainingInfo->fileName = "/test/FsFileWatcherMockTest_StopNotify_007";
    remainingInfo->events = IN_CREATE;
    remainingInfo->wd = expectedWd;
    watcher.dataCache_.AddWatcherInfo(remainingInfo);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(expectedWd));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_007";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_008
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when having remainingEvents but
 * NotifyToWatchNewEvents fails for inotify_add_watch fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_008";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_008";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set having remainingEvents condition
    auto remainingInfo = std::make_shared<WatcherInfo>(nullptr);
    remainingInfo->fileName = "/test/FsFileWatcherMockTest_StopNotify_008";
    remainingInfo->events = IN_CREATE;
    remainingInfo->wd = expectedWd;
    watcher.dataCache_.AddWatcherInfo(remainingInfo);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_008";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_009
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when having remainingEvents but
 * NotifyToWatchNewEvents fails for inotify_add_watch return another wd.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_009";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_009";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.dataCache_.AddWatcherInfo(info);
    // Set having remainingEvents condition
    auto remainingInfo = std::make_shared<WatcherInfo>(nullptr);
    remainingInfo->fileName = "/test/FsFileWatcherMockTest_StopNotify_009";
    remainingInfo->events = IN_CREATE;
    remainingInfo->wd = expectedWd;
    watcher.dataCache_.AddWatcherInfo(remainingInfo);
    // Set mock behaviors
    int32_t unexpectedWd = 200;
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(unexpectedWd));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_009";
}

/**
 * @tc.name: FsFileWatcherMockTest_StopNotify_010
 * @tc.desc: Test function of FsFileWatcher::StopNotify interface for FAILURE when CloseNotifyFd fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_StopNotify_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_StopNotify_010";
    // Prepare test parameters
    int32_t expectedWd = 100;
    auto info = std::make_shared<WatcherInfo>(nullptr);
    info->fileName = "/test/FsFileWatcherMockTest_StopNotify_010";
    info->events = IN_DELETE;
    info->wd = expectedWd;
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    watcher.eventFd_ = 1;
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*unistdMock, access(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(2).WillRepeatedly(testing::Return(EIO));
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    // Do testing
    int32_t result = watcher.StopNotify(info);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(result, EIO);
    EXPECT_EQ(watcher.notifyFd_, -1);
    EXPECT_EQ(watcher.eventFd_, -1);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_StopNotify_010";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_001
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when run_ is true.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_001";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = true;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_)).Times(0);
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_002
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret > 0 and fds[0].revents has
 * POLLNVAL.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_002";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([](struct pollfd *fds, nfds_t n, int timeout) {
            fds[0].revents = POLLNVAL;
            return 1;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_003
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret > 0 and fds[0].revents has
 * POLLIN.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([](struct pollfd *fds, nfds_t n, int timeout) {
            fds[0].revents = POLLIN;
            return 1;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_004
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret > 0 and fds[1].revents has
 * POLLIN.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_004";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    watcher.closed_ = true; // Avoid calling ReadNotifyEvent
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([&watcher](struct pollfd *fds, nfds_t n, int timeout) {
            fds[1].revents = POLLIN;
            watcher.run_ = false; // Ensure the loop will exit
            return 1;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_004";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_005
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret == 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_005";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([&watcher](struct pollfd *fds, nfds_t n, int timeout) {
            watcher.run_ = false; // Ensure the loop will exit
            return 0;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_005";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_006
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret < 0 and errno == EINTR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_006";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([&watcher](struct pollfd *fds, nfds_t n, int timeout) {
            errno = EINTR;
            watcher.run_ = false; // Ensure the loop will exit
            return -1;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_006";
}

/**
 * @tc.name: FsFileWatcherMockTest_GetNotifyEvent_007
 * @tc.desc: Test function of FsFileWatcher::GetNotifyEvent interface when poll returns ret < 0 and errno != EINTR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_GetNotifyEvent_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_GetNotifyEvent_007";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 2;
    // Set mock behaviors
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([&watcher](struct pollfd *fds, nfds_t n, int timeout) {
            errno = EIO;
            watcher.run_ = false; // Ensure the loop will exit
            return -1;
        });
    // Do testing
    watcher.GetNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_FALSE(watcher.run_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_GetNotifyEvent_007";
}

/**
 * @tc.name: FsFileWatcherMockTest_ReadNotifyEventLocked_001
 * @tc.desc: Test ReadNotifyEventLocked when closed_ is false.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_ReadNotifyEventLocked_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_ReadNotifyEventLocked_001";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.closed_ = false;
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);
    // Do testing
    watcher.ReadNotifyEventLocked();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(watcher.reading_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_ReadNotifyEventLocked_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_ReadNotifyEventLocked_002
 * @tc.desc: Test ReadNotifyEventLocked when close after read.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_ReadNotifyEventLocked_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_ReadNotifyEventLocked_002";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.closed_ = false;
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce([&watcher](int fd, void *buf, size_t count) {
            errno = EIO;
            watcher.closed_ = true; // Set close after read condition
            return 0;
        });
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(2).WillRepeatedly(testing::Return(0));
    // Do testing
    watcher.ReadNotifyEventLocked();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(watcher.closed_);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_ReadNotifyEventLocked_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_ReadNotifyEvent_001
 * @tc.desc: Test function of FsFileWatcher::ReadNotifyEvent interface for SUCCESS when read valid event data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_ReadNotifyEvent_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_ReadNotifyEvent_001";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    int32_t len = static_cast<int32_t>(sizeof(struct inotify_event));
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(len));
    // Do testing
    watcher.ReadNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_ReadNotifyEvent_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_ReadNotifyEvent_002
 * @tc.desc: Test function of FsFileWatcher::ReadNotifyEvent interface for FAILURE when read returns -1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_ReadNotifyEvent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_ReadNotifyEvent_002";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    watcher.ReadNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_EQ(errno, EIO);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_ReadNotifyEvent_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_ReadNotifyEvent_003
 * @tc.desc: Test function of FsFileWatcher::ReadNotifyEvent interface for SUCCESS when read returns 0 (EOF).
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_ReadNotifyEvent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_ReadNotifyEvent_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Set mock behaviors
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(0, 0));
    // Do testing
    watcher.ReadNotifyEvent();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_EQ(errno, 0);
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_ReadNotifyEvent_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_NotifyEvent_001
 * @tc.desc: Test function of FsFileWatcher::NotifyEvent interface for SUCCESS when valid event without filename.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_NotifyEvent_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_NotifyEvent_001";
    // Prepare test parameters
    int32_t expectedWd = 100;
    uint32_t mask = IN_CREATE;
    struct inotify_event event = { .wd = expectedWd, .mask = mask, .cookie = 0, .len = 0 };
    // Prepare test condition
    auto callback = std::make_shared<MockWatcherCallback>();
    auto info = std::make_shared<WatcherInfo>(callback);
    info->fileName = "/test/FsFileWatcherMockTest_NotifyEvent_001";
    info->events = mask;
    info->wd = expectedWd;
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    EXPECT_CALL(*callback, InvokeCallback(testing::_, testing::_, testing::_)).Times(1);
    // Do testing
    watcher.NotifyEvent(&event);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(callback.get());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_NotifyEvent_001";
}

/**
 * @tc.name: FsFileWatcherMockTest_NotifyEvent_002
 * @tc.desc: Test function of FsFileWatcher::NotifyEvent interface for SUCCESS when valid event with filename.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_NotifyEvent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_NotifyEvent_002";
    // Prepare test parameters
    int32_t expectedWd = 100;
    const char *name = "test.txt";
    size_t len = strlen(name);
    uint32_t mask = IN_CREATE;
    size_t totalSize = sizeof(struct inotify_event) + len + 1;
    std::vector<char> buffer(totalSize);
    struct inotify_event *event = reinterpret_cast<struct inotify_event *>(buffer.data());
    event->wd = expectedWd;
    event->mask = mask;
    event->cookie = 0;
    event->len = len + 1;
    char *namePtr = reinterpret_cast<char *>(event + 1);
    int ret = memcpy_s(namePtr, len + 1, name, len + 1);
    if (ret != 0) {
        EXPECT_EQ(ret, 0);
        GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_NotifyEvent_002";
        return;
    }
    // Prepare test condition
    auto callback = std::make_shared<MockWatcherCallback>();
    auto info = std::make_shared<WatcherInfo>(callback);
    info->fileName = "/test/FsFileWatcherMockTest_NotifyEvent_002";
    info->events = mask;
    info->wd = expectedWd;
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    EXPECT_CALL(*callback, InvokeCallback(testing::_, testing::_, testing::_)).Times(1);
    // Do testing
    watcher.NotifyEvent(event);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(callback.get());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_NotifyEvent_002";
}

/**
 * @tc.name: FsFileWatcherMockTest_NotifyEvent_003
 * @tc.desc: Test function of FsFileWatcher::NotifyEvent interface for FAILURE when event pointer is NULL.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_NotifyEvent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_NotifyEvent_003";
    // Prepare test condition
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    // Do testing
    watcher.NotifyEvent(nullptr);
    // Verify results
    EXPECT_TRUE(watcher.dataCache_.wdFileNameCache_.empty());
    EXPECT_TRUE(watcher.dataCache_.watcherInfoCache_.empty());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_NotifyEvent_003";
}

/**
 * @tc.name: FsFileWatcherMockTest_NotifyEvent_004
 * @tc.desc: Test function of FsFileWatcher::NotifyEvent interface for FAILURE when no matched watcher found.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileWatcherMockTest, FsFileWatcherMockTest_NotifyEvent_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-begin FsFileWatcherMockTest_NotifyEvent_004";
    // Prepare test parameters
    int32_t expectedWd = 100;
    int32_t unexpectedWd = 200;
    struct inotify_event event = { .wd = expectedWd, .mask = IN_CREATE, .cookie = 0, .len = 0 };
    // Prepare test condition
    auto callback = std::make_shared<MockWatcherCallback>();
    auto info = std::make_shared<WatcherInfo>(callback);
    info->fileName = "/test/FsFileWatcherMockTest_NotifyEvent_004";
    info->events = IN_MODIFY; // Not matched mask
    info->wd = unexpectedWd;  // Not matched wd
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    EXPECT_CALL(*callback, InvokeCallback(testing::_, testing::_, testing::_)).Times(0);
    // Do testing
    watcher.NotifyEvent(&event);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(callback.get());
    EXPECT_FALSE(watcher.dataCache_.wdFileNameCache_.empty());
    EXPECT_FALSE(watcher.dataCache_.watcherInfoCache_.empty());
    GTEST_LOG_(INFO) << "FsFileWatcherMockTest-end FsFileWatcherMockTest_NotifyEvent_004";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
