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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "eventfd_mock.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"
#include "inotify_mock.h"
#include "mock_watcher_callback.h"
#include "watcher_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class WatcherCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatcherCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void WatcherCoreMockTest::TearDownTestCase(void)
{
    EventfdMock::DestroyMock();
    InotifyMock::DestroyMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WatcherCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void WatcherCoreMockTest::TearDown(void)
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
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_001
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_001";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_001";
    int32_t events = IN_CREATE;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    auto eventfdMock = EventfdMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::Return(1));
    EXPECT_CALL(*eventfdMock, eventfd(testing::_, testing::_)).Times(1).WillOnce(testing::Return(2));
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(eventfdMock.get());
    EXPECT_TRUE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_001";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_002
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when InitNotify fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_002";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_002";
    int32_t events = IN_CREATE;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::SetErrnoAndReturn(EIO, -1));
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_FALSE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_002";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_003
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when events are invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_003";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_003";
    int32_t events = -1;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_003";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_004
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when CheckEventValid false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_004";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_004";
    int32_t invalidEvents = ~IN_ALL_EVENTS;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, invalidEvents, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_004";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_005
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when callback is null.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_005";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_005";
    int32_t events = IN_CREATE;
    std::shared_ptr<MockWatcherCallback> callback = nullptr;
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_005";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_006
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when AddWatcherInfo fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_006";
    // Prepare test parameters
    std::string path = "/test/WatcherCoreMockTest_DoCreateWatcher_006";
    int32_t events = IN_CREATE;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Prepare test condition
    int32_t expectedWd = 100;
    FsFileWatcher &watcher = FsFileWatcher::GetInstance();
    watcher.notifyFd_ = 1; // Valid notifyFd
    auto info = std::make_shared<WatcherInfo>(callback);
    info->fileName = "/test/WatcherCoreMockTest_DoCreateWatcher_006";
    info->events = IN_CREATE;
    info->wd = expectedWd;
    watcher.dataCache_.AddWatcherInfo(info);
    // Set mock behaviors
    auto cbMock = std::dynamic_pointer_cast<MockWatcherCallback>(callback);
    EXPECT_CALL(*cbMock, IsStrictEquals(testing::_)).Times(1).WillOnce(testing::Return(true));
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    testing::Mock::VerifyAndClearExpectations(cbMock.get());
    EXPECT_FALSE(result.IsSuccess());
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_006";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS