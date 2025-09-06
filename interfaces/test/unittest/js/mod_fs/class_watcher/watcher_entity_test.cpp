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

#include "watcher_entity.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "eventfd_mock.h"
#include "inotify_mock.h"
#include "unistd_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {

class WatcherEntityTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatcherEntityTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void WatcherEntityTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WatcherEntityTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    EventfdMock::EnableMock();
    InotifyMock::EnableMock();
    UnistdMock::EnableMock();
    errno = 0;
}

void WatcherEntityTest::TearDown(void)
{
    // Reset all mocks
    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    UnistdMock::DisableMock();
    // Reset FileWatcher stat
    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.reading_ = false;
    watcher.closed_ = false;
    watcher.notifyFd_ = -1;
    watcher.eventFd_ = -1;
    watcher.watcherInfoSet_.clear();
    watcher.wdFileNameMap_.clear();
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t INITIALIZED_NOTIFYFD = 1;
inline const int32_t UNINITIALIZED_NOTIFYFD = -1;
inline const int32_t INITIALIZED_EVENTFD = 1;
inline const int32_t UNINITIALIZED_EVENTFD = -1;

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFd_001
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFd interface for SUCCESS without close.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFd_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFd_001";

    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    auto ret = watcher.CloseNotifyFd();
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFd_001";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFd_002
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFd interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFd_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFd_002";

    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    UnistdMock::DisableMock();
    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = true;
    watcher.InitNotify();
    auto ret = watcher.CloseNotifyFd();
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFd_002";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFd_003
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFd interface for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFd_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFd_003";

    UnistdMock::DisableMock();
    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = true;
    auto ret = watcher.CloseNotifyFd();
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFd_003";
}

/**
 * @tc.name: WatcherEntityTest_InitNotify_001
 * @tc.desc: Test function of FileWatcher::InitNotify interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_InitNotify_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_InitNotify_001";

    // Prepare test condition
    FileWatcher &watcher = FileWatcher::GetInstance();
    // Set mock behaviors
    auto eventfdMock = EventfdMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::Return(INITIALIZED_NOTIFYFD));
    EXPECT_CALL(*eventfdMock, eventfd(testing::_, testing::_)).Times(1).WillOnce(testing::Return(INITIALIZED_EVENTFD));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(eventfdMock.get());
    EXPECT_TRUE(result);
    EXPECT_EQ(watcher.notifyFd_, INITIALIZED_NOTIFYFD);
    EXPECT_EQ(watcher.eventFd_, INITIALIZED_EVENTFD);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_InitNotify_001";
}

/**
 * @tc.name: WatcherEntityTest_InitNotify_002
 * @tc.desc: Test function of FileWatcher::InitNotify interface for FAILURE when inotify_init fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_InitNotify_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_InitNotify_002";

    // Prepare test condition
    FileWatcher &watcher = FileWatcher::GetInstance();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init())
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, UNINITIALIZED_NOTIFYFD));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_FALSE(result);
    EXPECT_EQ(watcher.notifyFd_, UNINITIALIZED_EVENTFD);
    EXPECT_EQ(watcher.eventFd_, UNINITIALIZED_EVENTFD);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_InitNotify_002";
}

/**
 * @tc.name: WatcherEntityTest_InitNotify_003
 * @tc.desc: Test function of FileWatcher::InitNotify interface for FAILURE when eventfd fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_InitNotify_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_InitNotify_003";

    // Prepare test condition
    FileWatcher &watcher = FileWatcher::GetInstance();
    // Set mock behaviors
    auto inotifyMock = InotifyMock::GetMock();
    auto eventfdMock = EventfdMock::GetMock();
    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*inotifyMock, inotify_init()).Times(1).WillOnce(testing::Return(INITIALIZED_NOTIFYFD));
    EXPECT_CALL(*eventfdMock, eventfd(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::SetErrnoAndReturn(EIO, UNINITIALIZED_EVENTFD));
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(1).WillOnce(testing::Return(0));
    // Do testing
    bool result = watcher.InitNotify();
    // Verify results
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    testing::Mock::VerifyAndClearExpectations(eventfdMock.get());
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(result);
    EXPECT_EQ(watcher.notifyFd_, UNINITIALIZED_EVENTFD);
    EXPECT_EQ(watcher.eventFd_, UNINITIALIZED_EVENTFD);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_InitNotify_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test