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

#include <gtest/gtest.h>
#include <thread>

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
}

void WatcherEntityTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

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

    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = true;
    auto ret = watcher.CloseNotifyFd();
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_CloseNotifyFd_003";
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

    UnistdMock::EnableMock();
    auto &watcher = FileWatcher::GetInstance();
    int32_t len = static_cast<int32_t>(sizeof(struct inotify_event));
    WatcherCallback callback = nullptr;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(len));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    UnistdMock::DisableMock();

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

    UnistdMock::EnableMock();
    auto &watcher = FileWatcher::GetInstance();
    int32_t eventSize = static_cast<int32_t>(sizeof(struct inotify_event));
    int32_t partialSize = eventSize - 4;
    WatcherCallback callback = nullptr;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(partialSize));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    UnistdMock::DisableMock();

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

    UnistdMock::EnableMock();
    auto &watcher = FileWatcher::GetInstance();
    int32_t eventSize = static_cast<int32_t>(sizeof(struct inotify_event));
    uint32_t nameLen = 20;
    int32_t bufferSize = eventSize + 10;
    WatcherCallback callback = nullptr;

    char buf[BUF_SIZE] = {0};
    struct inotify_event* mockEvent = reinterpret_cast<inotify_event*>(buf);
    mockEvent->wd = 1;
    mockEvent->len = nameLen;

    auto unistdMock = UnistdMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::Return(bufferSize));

    watcher.ReadNotifyEvent(callback);
    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    UnistdMock::DisableMock();

    GTEST_LOG_(INFO) << "WatcherEntityTest-end WatcherEntityTest_ReadNotifyEvent_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test