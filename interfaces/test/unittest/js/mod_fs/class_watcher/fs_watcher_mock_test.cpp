/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "fs_watcher.h"

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"
#include "inotify_mock.h"
#include "unistd_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class FsWatcherMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsWatcherMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsWatcherMockTest");
    InotifyMock::EnableMock();
    UnistdMock::EnableMock();
}

void FsWatcherMockTest::TearDownTestSuite()
{
    InotifyMock::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsWatcherMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void FsWatcherMockTest::TearDown()
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
 * @tc.name: FsWatcherTest_Constructor_001
 * @tc.desc: Test function of FsWatcher::Constructor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Constructor_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Constructor_001";
    // Do testing
    auto result = FsWatcher::Constructor();
    // Verify results
    ASSERT_TRUE(result.IsSuccess());
    std::unique_ptr<FsWatcher> watcher(result.GetData().value()); // To smart ptr for auto memory release

    ASSERT_NE(watcher, nullptr);
    EXPECT_NE(watcher->GetWatchEntity(), nullptr);
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Constructor_001";
}

/**
 * @tc.name: FsWatcherTest_Start_001
 * @tc.desc: Test function of FsWatcher::Start interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Start_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Start_001";
    // Prepare test condition
    int32_t expectedWd = 100;
    auto watchEntity = CreateUniquePtr<FsWatchEntity>();
    FsWatcher fsWatcher(std::move(watchEntity));
    std::shared_ptr<WatcherInfo> info = std::make_shared<WatcherInfo>(nullptr);
    fsWatcher.GetWatchEntity()->watcherInfo = info;
    // Prepare test condition for FsFileWatcher
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1;       // Valid notifyFd
    watcher.taskRunning_ = true; // Avoid starting thread
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_add_watch(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(expectedWd));
    // Do testing
    auto result = fsWatcher.Start();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_TRUE(result.IsSuccess());
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Start_001";
}

/**
 * @tc.name: FsWatcherTest_Start_002
 * @tc.desc: Test function of FsWatcher::Start interface for FAILURE when watchEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Start_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Start_002";
    // Prepare test condition
    FsWatcher fsWatcher(nullptr);
    // Do testing
    auto result = fsWatcher.Start();
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Start_002";
}

/**
 * @tc.name: FsWatcherTest_Start_003
 * @tc.desc: Test function of FsWatcher::Start interface for FAILURE when StartNotify fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Start_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Start_003";
    // Prepare test condition
    auto watchEntity = CreateUniquePtr<FsWatchEntity>();
    FsWatcher fsWatcher(std::move(watchEntity));
    std::shared_ptr<WatcherInfo> info = std::make_shared<WatcherInfo>(nullptr);
    fsWatcher.GetWatchEntity()->watcherInfo = info;
    // Prepare test condition for FsFileWatcher
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = -1; // Invalid notifyFd
    // Do testing
    auto result = fsWatcher.Start();
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Start_003";
}

/**
 * @tc.name: FsWatcherTest_Stop_001
 * @tc.desc: Test function of FsWatcher::Stop interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Stop_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Stop_001";
    // Prepare test condition
    auto watchEntity = CreateUniquePtr<FsWatchEntity>();
    FsWatcher fsWatcher(std::move(watchEntity));
    std::shared_ptr<WatcherInfo> info = std::make_shared<WatcherInfo>(nullptr);
    fsWatcher.GetWatchEntity()->watcherInfo = info;
    // Prepare test condition for FsFileWatcher
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1;
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_rm_watch(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(0, 0));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(2).WillRepeatedly(testing::Return(0));
    // Do testing
    auto result = fsWatcher.Stop();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_TRUE(result.IsSuccess());
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Stop_001";
}

/**
 * @tc.name: FsWatcherTest_Stop_002
 * @tc.desc: Test function of FsWatcher::Stop interface for FAILURE when watchEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Stop_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Stop_002";
    // Prepare test condition
    FsWatcher fsWatcher(nullptr);
    // Do testing
    auto result = fsWatcher.Stop();
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Stop_002";
}

/**
 * @tc.name: FsWatcherTest_Stop_003
 * @tc.desc: Test function of FsWatcher::Stop interface for FAILURE when StopNotify fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsWatcherMockTest, FsWatcherTest_Stop_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsWatcherMockTest-begin FsWatcherTest_Stop_003";
    // Prepare test condition
    auto watchEntity = CreateUniquePtr<FsWatchEntity>();
    FsWatcher fsWatcher(std::move(watchEntity));
    std::shared_ptr<WatcherInfo> info = std::make_shared<WatcherInfo>(nullptr);
    fsWatcher.GetWatchEntity()->watcherInfo = info;
    // Prepare test condition for FsFileWatcher
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = -1; // Invalid notifyFd
    // Do testing
    auto result = fsWatcher.Stop();
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    GTEST_LOG_(INFO) << "FsWatcherMockTest-end FsWatcherTest_Stop_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS