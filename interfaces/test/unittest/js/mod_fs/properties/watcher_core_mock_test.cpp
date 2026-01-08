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

#include "watcher_core.h"

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "eventfd_mock.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"
#include "inotify_mock.h"
#include "mock_watcher_callback.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class WatcherCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void WatcherCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "WatcherCoreMockTest");
    EventfdMock::EnableMock();
    InotifyMock::EnableMock();
}

void WatcherCoreMockTest::TearDownTestSuite()
{
    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void WatcherCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void WatcherCoreMockTest::TearDown()
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
    std::string path = "fakePath/WatcherCoreMockTest_DoCreateWatcher_001";
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
    ASSERT_TRUE(result.IsSuccess());
    std::unique_ptr<FsWatcher> watcher(result.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(watcher, nullptr);
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_001";
}

/**
 * @tc.name: WatcherCoreMockTest_DoCreateWatcher_002
 * @tc.desc: Test function of WatcherCore::DoCreateWatcher interface for FAILURE when TryInitNotify fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherCoreMockTest, WatcherCoreMockTest_DoCreateWatcher_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-begin WatcherCoreMockTest_DoCreateWatcher_002";
    // Prepare test parameters
    std::string path = "fakePath/WatcherCoreMockTest_DoCreateWatcher_002";
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
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
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
    std::string path = "fakePath/WatcherCoreMockTest_DoCreateWatcher_003";
    int32_t events = -1;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
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
    std::string path = "fakePath/WatcherCoreMockTest_DoCreateWatcher_004";
    int32_t invalidEvents = ~IN_ALL_EVENTS;
    std::shared_ptr<MockWatcherCallback> callback = std::make_shared<MockWatcherCallback>();
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, invalidEvents, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
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
    std::string path = "fakePath/WatcherCoreMockTest_DoCreateWatcher_005";
    int32_t events = IN_CREATE;
    std::shared_ptr<MockWatcherCallback> callback = nullptr;
    // Do testing
    auto result = WatcherCore::DoCreateWatcher(path, events, callback);
    // Verify results
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    GTEST_LOG_(INFO) << "WatcherCoreMockTest-end WatcherCoreMockTest_DoCreateWatcher_005";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS