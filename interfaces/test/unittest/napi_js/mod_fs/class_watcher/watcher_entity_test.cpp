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

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "eventfd_mock.h"
#include "inotify_mock.h"
#include "poll_mock.h"
#include "unistd_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {

class WatcherEntityTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void WatcherEntityTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "WatcherEntityTest");
}

void WatcherEntityTest::TearDownTestSuite(void)
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void WatcherEntityTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    EventfdMock::EnableMock();
    InotifyMock::EnableMock();
    PollMock::EnableMock();
    UnistdMock::EnableMock();
    errno = 0;
}

void WatcherEntityTest::TearDown(void)
{
    // Reset all mocks
    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    PollMock::DisableMock();
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
inline const int32_t TOTAL_THREADS = 20;

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

/**
 * @tc.name: WatcherEntityTest_ReadNotifyEvent_001
 * @tc.desc: Test function of WatcherEntityTest::ReadNotifyEvent interface for SUCCESS when read valid event data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_ReadNotifyEvent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_ReadNotifyEvent_001";

    auto &watcher = FileWatcher::GetInstance();
    int32_t len = static_cast<int32_t>(sizeof(struct inotify_event));
    WatcherCallback callback = nullptr;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(len));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_ReadNotifyEvent_001";
}

/**
 * @tc.name: WatcherEntityTest_ReadNotifyEvent_002
 * @tc.desc: Test first if branch - remaining data < sizeof(inotify_event)
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_ReadNotifyEvent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_ReadNotifyEvent_002";

    auto &watcher = FileWatcher::GetInstance();
    int32_t eventSize = static_cast<int32_t>(sizeof(struct inotify_event));
    int32_t partialSize = eventSize - 4;
    WatcherCallback callback = nullptr;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(partialSize));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_ReadNotifyEvent_002";
}

/**
 * @tc.name: WatcherEntityTest_ReadNotifyEvent_003
 * @tc.desc: Test second if branch - event->len exceeds available buffer
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_ReadNotifyEvent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_ReadNotifyEvent_003";

    auto &watcher = FileWatcher::GetInstance();
    int32_t eventSize = static_cast<int32_t>(sizeof(struct inotify_event));
    uint32_t nameLen = 20;
    int32_t bufferSize = eventSize + 10;
    WatcherCallback callback = nullptr;

    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *mockEvent = reinterpret_cast<inotify_event *>(buf);
    mockEvent->wd = 1;
    mockEvent->len = nameLen;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(bufferSize));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_ReadNotifyEvent_003";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_001
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for SUCCESS without close.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_001";

    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.reading_ = false;
    auto ret = watcher.CloseNotifyFdLocked();
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_001";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_002
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for SUCCESS without close.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_002";

    auto &watcher = FileWatcher::GetInstance();
    watcher.reading_ = true;
    auto ret = watcher.CloseNotifyFdLocked();
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_002";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_003
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for multi.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_003";

    auto &watcher = FileWatcher::GetInstance();
    watcher.reading_ = true;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 1;
    std::vector<std::thread> threads;
    threads.reserve(TOTAL_THREADS);

    for (int i = 0; i < TOTAL_THREADS; ++i) {
        threads.emplace_back([&watcher]() {
            int result = watcher.CloseNotifyFdLocked();
            EXPECT_EQ(result, 0);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(watcher.closed_);
    EXPECT_EQ(watcher.notifyFd_, 1);
    EXPECT_EQ(watcher.eventFd_, 1);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_003";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_004
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for multi.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_004";

    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.reading_ = false;
    watcher.notifyFd_ = 1;
    watcher.eventFd_ = 1;
    std::vector<std::thread> threads;
    threads.reserve(TOTAL_THREADS);

    for (int i = 0; i < TOTAL_THREADS; ++i) {
        threads.emplace_back([&watcher]() {
            int result = watcher.CloseNotifyFdLocked();
            EXPECT_EQ(result, 0);
        });
    }

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, close(testing::_)).Times(0);

    for (auto& t : threads) {
        t.join();
    }

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    EXPECT_FALSE(watcher.closed_);
    EXPECT_EQ(watcher.notifyFd_, 1);
    EXPECT_EQ(watcher.eventFd_, 1);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_004";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_005
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for multi.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_005";

    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    UnistdMock::DisableMock();
    auto &watcher = FileWatcher::GetInstance();
    watcher.InitNotify();
    watcher.run_ = true;
    watcher.reading_ = false;
    watcher.watcherInfoSet_.clear();
    EXPECT_GT(watcher.notifyFd_, -1);
    EXPECT_GT(watcher.eventFd_, -1);
    std::vector<std::thread> threads;
    threads.reserve(TOTAL_THREADS);

    for (int i = 0; i < TOTAL_THREADS; ++i) {
        threads.emplace_back([&watcher]() {
            int result = watcher.CloseNotifyFdLocked();
            EXPECT_EQ(result, 0);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(watcher.notifyFd_, -1);
    EXPECT_EQ(watcher.eventFd_, -1);
    EXPECT_FALSE(watcher.run_);
    EXPECT_FALSE(watcher.closed_);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_005";
}

/**
 * @tc.name: WatcherEntityTest_CloseNotifyFdLocked_006
 * @tc.desc: Test function of WatcherEntityTest::CloseNotifyFdLocked interface for multi.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherEntityTest, WatcherEntityTest_CloseNotifyFdLocked_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherEntityTest-begin WatcherEntityTest_CloseNotifyFdLocked_006";

    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    UnistdMock::DisableMock();
    auto &watcher = FileWatcher::GetInstance();
    watcher.InitNotify();
    watcher.run_ = false;
    watcher.reading_ = false;
    EXPECT_GT(watcher.notifyFd_, -1);
    EXPECT_GT(watcher.eventFd_, -1);
    WatcherCallback callback = nullptr;
    std::vector<std::thread> threads;
    threads.reserve(TOTAL_THREADS);
    
    std::atomic<int> count(0);
    std::atomic<bool> started(false);
    for (int i = 0; i < TOTAL_THREADS; ++i) {
        threads.emplace_back([&watcher, &count, &started]() {
            count++;
            while (!started && (count < TOTAL_THREADS || !watcher.run_)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            started = true;
            watcher.reading_ = false;
            watcher.watcherInfoSet_.clear();
            int result = watcher.CloseNotifyFdLocked();
            EXPECT_EQ(result, 0);
        });
    }
    UnistdMock::EnableMock();
    auto uniMock = UnistdMock::GetMock();
    auto pollMock = PollMock::GetMock();
    EXPECT_CALL(*uniMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(-1));
    EXPECT_CALL(*pollMock, poll(testing::_, testing::_, testing::_))
        .WillRepeatedly([&watcher](struct pollfd *fds, nfds_t n, int timeout) {
            fds[1].revents = POLLIN;
            return 1;
        });
    watcher.GetNotifyEvent(callback);

    for (auto& t : threads) {
        t.join();
    }

    testing::Mock::VerifyAndClearExpectations(pollMock.get());
    EXPECT_EQ(watcher.notifyFd_, -1);
    EXPECT_EQ(watcher.eventFd_, -1);
    EXPECT_FALSE(watcher.run_);
    EXPECT_FALSE(watcher.closed_);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFdLocked_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test