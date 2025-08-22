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

#include <gtest/gtest.h>
#include <sys/inotify.h>

#include "filemgmt_libhilog.h"
#include "watcher_data_cache.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class WatcherDataCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatcherDataCacheTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void WatcherDataCacheTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WatcherDataCacheTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    errno = 0; // Reset errno
}

void WatcherDataCacheTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t EXPECTED_WD = 100;
inline const int32_t UNEXPECTED_WD = 200;
inline const uint32_t EXPECTED_EVENTS = IN_ACCESS | IN_CREATE;

/**
 * @tc.name: WatcherDataCacheTest_RemoveWatchedEvents_001
 * @tc.desc: Test function of WatcherDataCache::RemoveWatchedEvents for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_RemoveWatchedEvents_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_RemoveWatchedEvents_001";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_RemoveWatchedEvents_001";
    cache.wdFileNameCache_[fileName] = std::make_pair(EXPECTED_WD, IN_ACCESS);
    // Do testing
    cache.RemoveWatchedEvents(fileName);
    // Verify results
    EXPECT_TRUE(cache.wdFileNameCache_.empty());
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_RemoveWatchedEvents_001";
}

/**
 * @tc.name: WatcherDataCacheTest_RemoveWatchedEvents_002
 * @tc.desc: Test function of WatcherDataCache::RemoveWatchedEvents for SUCCESS when key(fileName) not exist.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_RemoveWatchedEvents_002, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_RemoveWatchedEvents_002";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_RemoveWatchedEvents_002";
    cache.wdFileNameCache_[fileName] = std::make_pair(EXPECTED_WD, IN_ACCESS);
    // Do testing
    cache.RemoveWatchedEvents("fileName_not_exist");
    // Verify results
    EXPECT_EQ(cache.wdFileNameCache_.size(), 1);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_RemoveWatchedEvents_002";
}

/**
 * @tc.name: WatcherDataCacheTest_UpdateWatchedEvents_001
 * @tc.desc: Test function of WatcherDataCache::UpdateWatchedEvents for SUCCESS when key(fileName) exist.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_UpdateWatchedEvents_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_UpdateWatchedEvents_001";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_UpdateWatchedEvents_001";
    cache.wdFileNameCache_[fileName] = std::make_pair(UNEXPECTED_WD, IN_DELETE);
    // Do testing
    cache.UpdateWatchedEvents(fileName, EXPECTED_WD, EXPECTED_EVENTS);
    // Verify results
    EXPECT_EQ(cache.wdFileNameCache_.size(), 1);
    EXPECT_EQ(cache.wdFileNameCache_[fileName].first, EXPECTED_WD);
    EXPECT_EQ(cache.wdFileNameCache_[fileName].second, EXPECTED_EVENTS);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_UpdateWatchedEvents_001";
}

/**
 * @tc.name: WatcherDataCacheTest_UpdateWatchedEvents_002
 * @tc.desc: Test function of WatcherDataCache::UpdateWatchedEvents for SUCCESS when key(fileName) not exist.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_UpdateWatchedEvents_002, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_UpdateWatchedEvents_002";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_UpdateWatchedEvents_002";
    // Do testing
    cache.UpdateWatchedEvents(fileName, EXPECTED_WD, EXPECTED_EVENTS);
    // Verify results
    EXPECT_EQ(cache.wdFileNameCache_.size(), 1);
    EXPECT_EQ(cache.wdFileNameCache_[fileName].first, EXPECTED_WD);
    EXPECT_EQ(cache.wdFileNameCache_[fileName].second, EXPECTED_EVENTS);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_UpdateWatchedEvents_002";
}

/**
 * @tc.name: WatcherDataCacheTest_FindWatchedEvents_001
 * @tc.desc: Test function of WatcherDataCache::FindWatchedEvents for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_FindWatchedEvents_001, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_FindWatchedEvents_001";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_FindWatchedEvents_001";
    cache.wdFileNameCache_[fileName] = std::make_pair(EXPECTED_WD, EXPECTED_EVENTS);
    // Do testing
    auto [isWatched, wd, events] = cache.FindWatchedEvents(fileName, IN_ACCESS);
    // Verify results
    EXPECT_TRUE(isWatched);
    EXPECT_EQ(wd, EXPECTED_WD);
    EXPECT_EQ(events, EXPECTED_EVENTS);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_FindWatchedEvents_001";
}

/**
 * @tc.name: WatcherDataCacheTest_FindWatchedEvents_002
 * @tc.desc: Test function of WatcherDataCache::FindWatchedEvents for FAILURE when key(fileName) not exist.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_FindWatchedEvents_002, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_FindWatchedEvents_002";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_FindWatchedEvents_002";
    // Do testing
    auto [isWatched, wd, events] = cache.FindWatchedEvents(fileName, IN_ACCESS);
    // Verify results
    EXPECT_FALSE(isWatched);
    EXPECT_EQ(wd, -1);
    EXPECT_EQ(events, 0);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_FindWatchedEvents_002";
}

/**
 * @tc.name: WatcherDataCacheTest_FindWatchedEvents_003
 * @tc.desc: Test function of WatcherDataCache::FindWatchedEvents for FAILURE when event unmatched.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 0
 */
HWTEST_F(WatcherDataCacheTest, WatcherDataCacheTest_FindWatchedEvents_003, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-begin WatcherDataCacheTest_FindWatchedEvents_003";
    // Prepare test condition
    WatcherDataCache cache;
    auto fileName = "fakePath/WatcherDataCacheTest_FindWatchedEvents_003";
    cache.wdFileNameCache_[fileName] = std::make_pair(EXPECTED_WD, IN_DELETE);
    // Do testing
    auto [isWatched, wd, events] = cache.FindWatchedEvents(fileName, IN_ACCESS);
    // Verify results
    EXPECT_FALSE(isWatched);
    EXPECT_EQ(wd, EXPECTED_WD);
    EXPECT_EQ(events, IN_DELETE);
    cache.wdFileNameCache_.clear();
    GTEST_LOG_(INFO) << "WatcherDataCacheTest-end WatcherDataCacheTest_FindWatchedEvents_003";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
