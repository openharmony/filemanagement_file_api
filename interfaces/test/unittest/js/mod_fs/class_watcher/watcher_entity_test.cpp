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

} // namespace OHOS::FileManagement::ModuleFileIO::Test