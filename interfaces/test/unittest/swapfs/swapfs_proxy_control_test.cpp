/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include <array>
#include <cstdint>

#include "ipc_object_stub.h"
#include "swapfs_proxy_control.h"
#include "swapfs_storage_manager_mock.h"

namespace {
using OHOS::FileManagement::Swapfs::DEFAULT_SPACE_CHECK_INTERVAL_MS;
using OHOS::FileManagement::Swapfs::DEFAULT_SPACE_LIMIT_BYTES;
using OHOS::FileManagement::Swapfs::MIN_AVAILABLE_DEVICE_SPACE_BYTES;
using OHOS::FileManagement::Swapfs::ProxySwapControlProvider;
using OHOS::FileManagement::Swapfs::Test::CreateStorageManagerProxyMock;
using OHOS::FileManagement::Swapfs::Test::TestStorageManagerProxy;

class TestableProxySwapControlProvider final : public ProxySwapControlProvider {
public:
    void SetRemoteObject(const OHOS::sptr<OHOS::IRemoteObject> &remote)
    {
        remote_ = remote;
    }

    void SetCastResult(const OHOS::sptr<OHOS::StorageManager::IStorageManager> &service)
    {
        service_ = service;
    }

    uint32_t castCount = 0;

private:
    OHOS::sptr<OHOS::IRemoteObject> GetStorageManagerRemote() override
    {
        return remote_;
    }

    OHOS::sptr<OHOS::StorageManager::IStorageManager> CastStorageManagerProxy(
        const OHOS::sptr<OHOS::IRemoteObject> &) override
    {
        ++castCount;
        return service_;
    }

    OHOS::sptr<OHOS::IRemoteObject> remote_;
    OHOS::sptr<OHOS::StorageManager::IStorageManager> service_;
};

class SwapfsProxyControlTest : public testing::Test {
protected:
    ProxySwapControlProvider provider;
};

struct ReserveCase {
    const char *name;
    uint64_t availableSpace;
    uint64_t limit;
    uint64_t committed;
    uint64_t reserved;
    uint64_t request;
    int expected;
    bool featureEnabled;
};

constexpr std::array<ReserveCase, 7> RESERVE_CASES {{
    { "low_space", MIN_AVAILABLE_DEVICE_SPACE_BYTES - 1, DEFAULT_SPACE_LIMIT_BYTES,
        0, 0, 4096, SWAPFS_E_NOSPC, false },
    { "quota_boundary", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192, 4096,
        4096, 0, 1, SWAPFS_E_QUOTA_EXCEEDED, true },
    { "quota_exceeded", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192, 4096,
        4097, 0, 0, SWAPFS_E_QUOTA_EXCEEDED, true },
    { "committed_exceeds_writable", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096,
        DEFAULT_SPACE_LIMIT_BYTES, 8192, 0, 0, SWAPFS_E_NOSPC, true },
    { "writable_boundary", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096,
        DEFAULT_SPACE_LIMIT_BYTES, 0, 0, 4096, SWAPFS_E_OK, true },
    { "writable_exceeded", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096,
        DEFAULT_SPACE_LIMIT_BYTES, 0, 0, 4097, SWAPFS_E_NOSPC, true },
    { "pending_exhausts_writable", MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096,
        DEFAULT_SPACE_LIMIT_BYTES, 0, 4096, 1, SWAPFS_E_NOSPC, true },
}};

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyInitUsesDefaultQuota_0000, testing::ext::TestSize.Level1)
{
    provider.InitWithAvailableSpace(MIN_AVAILABLE_DEVICE_SPACE_BYTES, 0, DEFAULT_SPACE_CHECK_INTERVAL_MS);

    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.spaceLimitBytes, DEFAULT_SPACE_LIMIT_BYTES);
    EXPECT_TRUE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NONE);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyReserveChecksQuotaAndSpaceThresholds_0000,
    testing::ext::TestSize.Level1)
{
    for (const auto &testCase : RESERVE_CASES) {
        SCOPED_TRACE(testCase.name);
        ProxySwapControlProvider current;
        current.InitWithAvailableSpace(
            testCase.availableSpace, testCase.limit, DEFAULT_SPACE_CHECK_INTERVAL_MS);
        if (testCase.committed > 0) {
            current.OnSwapOutCommitted(1, testCase.committed);
        }
        if (testCase.reserved > 0) {
            ASSERT_EQ(current.ReserveSwapOut(testCase.reserved), SWAPFS_E_OK);
        }
        EXPECT_EQ(current.ReserveSwapOut(testCase.request), testCase.expected);
        OH_SwapfsStats stats {};
        current.FillStats(stats);
        EXPECT_EQ(stats.featureEnabled, testCase.featureEnabled);
        EXPECT_EQ(stats.disableReason, testCase.featureEnabled ?
            OH_SWAPFS_DISABLE_REASON_NONE : OH_SWAPFS_DISABLE_REASON_NOSPC);
    }
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyTracksOccupiedWriteBytes_0000, testing::ext::TestSize.Level1)
{
    provider.InitWithAvailableSpace(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096, DEFAULT_SPACE_LIMIT_BYTES,
        DEFAULT_SPACE_CHECK_INTERVAL_MS);
    provider.OnSwapOutCommitted(1000, 4096);

    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 1);
    EXPECT_EQ(stats.totalDataSize, 1000);
    EXPECT_EQ(stats.totalOccupiedSize, 4096);
    EXPECT_EQ(stats.spaceLimitBytes, DEFAULT_SPACE_LIMIT_BYTES);
    EXPECT_TRUE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NONE);
    EXPECT_EQ(stats.accumulatedWriteBytes, 4096);
    EXPECT_EQ(stats.availableDeviceSpace, MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096);

    provider.OnEntryRemoved(1000, 4096);
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 0);
    EXPECT_EQ(stats.totalDataSize, 0);
    EXPECT_EQ(stats.totalOccupiedSize, 0);
    EXPECT_EQ(stats.accumulatedWriteBytes, 4096);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxySaturatesReservationAndEntryCounters_0000,
    testing::ext::TestSize.Level1)
{
    provider.InitWithAvailableSpace(
        MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192, 8192, DEFAULT_SPACE_CHECK_INTERVAL_MS);

    provider.CancelReservedSwapOut(4096);
    EXPECT_EQ(provider.pendingOccupiedSize_, 0u);
    EXPECT_EQ(provider.ReserveSwapOut(8192), SWAPFS_E_OK);
    EXPECT_EQ(provider.pendingOccupiedSize_, 8192u);
    provider.CancelReservedSwapOut(8192);
    EXPECT_EQ(provider.pendingOccupiedSize_, 0u);

    provider.OnEntryRemoved(1024, 4096);
    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 0);
    EXPECT_EQ(stats.totalDataSize, 0);
    EXPECT_EQ(stats.totalOccupiedSize, 0);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyReservationBlocksConcurrentQuotaOverflow_0000,
    testing::ext::TestSize.Level1)
{
    provider.InitWithAvailableSpace(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192, 8192, DEFAULT_SPACE_CHECK_INTERVAL_MS);

    EXPECT_EQ(provider.ReserveSwapOut(4096), SWAPFS_E_OK);
    EXPECT_EQ(provider.ReserveSwapOut(4097), SWAPFS_E_QUOTA_EXCEEDED);

    provider.CancelReservedSwapOut(4096);
    EXPECT_EQ(provider.ReserveSwapOut(4096), SWAPFS_E_OK);
    provider.OnSwapOutCommitted(1000, 4096);

    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 1);
    EXPECT_EQ(stats.totalOccupiedSize, 4096);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyOnAllEntriesRemoved_0000, testing::ext::TestSize.Level1)
{
    provider.InitWithAvailableSpace(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192, DEFAULT_SPACE_LIMIT_BYTES,
        DEFAULT_SPACE_CHECK_INTERVAL_MS);
    provider.OnSwapOutCommitted(2000, 4096);
    provider.OnSwapOutCommitted(3000, 4096);

    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 2);

    provider.OnAllEntriesRemoved();
    provider.FillStats(stats);
    EXPECT_EQ(stats.totalKeys, 0);
    EXPECT_EQ(stats.totalDataSize, 0);
    EXPECT_EQ(stats.totalOccupiedSize, 0);
}

// ============================ Init when proxy unavailable ============================

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyInitDisablesFeatureWhenProxyUnavailable_0000,
    testing::ext::TestSize.Level1)
{
    TestableProxySwapControlProvider p;
    ASSERT_EQ(p.Init(DEFAULT_SPACE_LIMIT_BYTES, DEFAULT_SPACE_CHECK_INTERVAL_MS), SWAPFS_E_OK);

    OH_SwapfsStats stats;
    p.FillStats(stats);
    EXPECT_EQ(stats.availableDeviceSpace, 0u);
    EXPECT_FALSE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NOSPC);
    EXPECT_EQ(stats.spaceLimitBytes, DEFAULT_SPACE_LIMIT_BYTES);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_GetOrCreateProxyHandlesSaAndBrokerStates_0000,
    testing::ext::TestSize.Level1)
{
    TestableProxySwapControlProvider current;
    EXPECT_EQ(current.GetOrCreateProxy(), nullptr);
    EXPECT_TRUE(current.proxyUnavailable_);

    current.SetRemoteObject(new OHOS::IPCObjectStub(u"swapfs.storage.test"));
    EXPECT_EQ(current.GetOrCreateProxy(), nullptr);
    EXPECT_TRUE(current.proxyUnavailable_);
    EXPECT_EQ(current.castCount, 1u);

    auto service = CreateStorageManagerProxyMock(0);
    current.SetCastResult(service);
    auto proxy = current.GetOrCreateProxy();
    ASSERT_NE(proxy, nullptr);
    EXPECT_EQ(proxy, service);
    EXPECT_FALSE(current.proxyUnavailable_);
    EXPECT_EQ(current.GetOrCreateProxy(), proxy);
    EXPECT_EQ(current.castCount, 2u);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRefreshKeepsOldValueWhenProxyUnavailable_0000,
    testing::ext::TestSize.Level1)
{
    ProxySwapControlProvider p;
    constexpr uint64_t initialSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192;
    p.InitWithAvailableSpace(initialSpace, DEFAULT_SPACE_LIMIT_BYTES, 0);
    p.proxyUnavailable_ = true;
    p.OnSwapOutCommitted(1024, 4096);

    EXPECT_EQ(p.ReserveSwapOut(4096), SWAPFS_E_OK);
    EXPECT_EQ(p.committedSinceSpaceCheckBytes_, 4096u);

    OH_SwapfsStats stats;
    p.FillStats(stats);
    EXPECT_EQ(stats.availableDeviceSpace, initialSpace);
    EXPECT_TRUE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NONE);
    EXPECT_EQ(stats.totalOccupiedSize, 4096u);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyReEnablesAfterSpaceRecovers_0000, testing::ext::TestSize.Level1)
{
    ProxySwapControlProvider p;
    int64_t freeSpace = 0;
    p.proxy_ = new TestStorageManagerProxy([&freeSpace](int64_t &value) {
        value = freeSpace;
        return 0;
    });
    ASSERT_EQ(p.Init(DEFAULT_SPACE_LIMIT_BYTES, 0), SWAPFS_E_OK);
    OH_SwapfsStats stats;
    p.FillStats(stats);
    ASSERT_FALSE(stats.featureEnabled);

    freeSpace = static_cast<int64_t>(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 4096);
    EXPECT_EQ(p.ReserveSwapOut(4096), SWAPFS_E_OK);

    p.FillStats(stats);
    EXPECT_TRUE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NONE);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRejectsInFlightCounterOverflow_0000,
    testing::ext::TestSize.Level1)
{
    ProxySwapControlProvider inFlightProvider;
    inFlightProvider.InitWithAvailableSpace(
        UINT64_MAX, UINT64_MAX, DEFAULT_SPACE_CHECK_INTERVAL_MS);
    inFlightProvider.pendingOccupiedSize_ = UINT64_MAX;
    inFlightProvider.committedSinceSpaceCheckBytes_ = 1;

    EXPECT_EQ(inFlightProvider.ReserveSwapOut(0), SWAPFS_E_NOSPC);
    EXPECT_EQ(inFlightProvider.pendingOccupiedSize_, UINT64_MAX);
    EXPECT_EQ(inFlightProvider.committedSinceSpaceCheckBytes_, 1u);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRefreshHonorsConfiguredInterval_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t initialSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192;
    provider.InitWithAvailableSpace(initialSpace, DEFAULT_SPACE_LIMIT_BYTES, DEFAULT_SPACE_CHECK_INTERVAL_MS);
    provider.proxyUnavailable_ = true;
    int64_t initialCheckTime = provider.lastSpaceCheckTime_;

    provider.RefreshAvailableSpaceIfNeeded();

    EXPECT_EQ(provider.lastSpaceCheckTime_, initialCheckTime);
    EXPECT_EQ(provider.availableDeviceSpace_, initialSpace);
    EXPECT_TRUE(provider.featureEnabled_);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRefreshRunsAfterConfiguredInterval_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t initialSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192;
    provider.InitWithAvailableSpace(initialSpace, DEFAULT_SPACE_LIMIT_BYTES, DEFAULT_SPACE_CHECK_INTERVAL_MS);
    provider.proxyUnavailable_ = true;
    provider.lastSpaceCheckTime_ = 0;

    provider.RefreshAvailableSpaceIfNeeded();

    EXPECT_GT(provider.lastSpaceCheckTime_, 0);
    EXPECT_EQ(provider.availableDeviceSpace_, initialSpace);
    EXPECT_TRUE(provider.featureEnabled_);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyInitUsesFreeSizeFromCachedProxy_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t freeSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 16384;
    provider.proxy_ = new TestStorageManagerProxy([freeSpace](int64_t &value) {
        value = static_cast<int64_t>(freeSpace);
        return 0;
    });

    EXPECT_EQ(
        provider.Init(DEFAULT_SPACE_LIMIT_BYTES, DEFAULT_SPACE_CHECK_INTERVAL_MS), SWAPFS_E_OK);
    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.availableDeviceSpace, freeSpace);
    EXPECT_TRUE(stats.featureEnabled);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyInitHandlesFreeSizeQueryFailure_0000,
    testing::ext::TestSize.Level1)
{
    provider.proxy_ = new TestStorageManagerProxy([](int64_t &) {
        return -1;
    });

    EXPECT_EQ(
        provider.Init(DEFAULT_SPACE_LIMIT_BYTES, DEFAULT_SPACE_CHECK_INTERVAL_MS), SWAPFS_E_OK);
    OH_SwapfsStats stats;
    provider.FillStats(stats);
    EXPECT_EQ(stats.availableDeviceSpace, 0u);
    EXPECT_FALSE(stats.featureEnabled);
    EXPECT_EQ(stats.disableReason, OH_SWAPFS_DISABLE_REASON_NOSPC);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRefreshUpdatesSpaceAndPreservesConcurrentCommit_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t initialSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192;
    constexpr uint64_t refreshedSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 16384;
    provider.InitWithAvailableSpace(initialSpace, DEFAULT_SPACE_LIMIT_BYTES, 0);
    provider.OnSwapOutCommitted(1024, 4096);
    provider.proxy_ = new TestStorageManagerProxy([&](int64_t &value) {
        provider.OnSwapOutCommitted(512, 2048);
        value = static_cast<int64_t>(refreshedSpace);
        return 0;
    });

    provider.RefreshAvailableSpaceIfNeeded();

    EXPECT_EQ(provider.availableDeviceSpace_, refreshedSpace);
    EXPECT_EQ(provider.committedSinceSpaceCheckBytes_, 2048u);
    EXPECT_TRUE(provider.featureEnabled_);
}

HWTEST_F(SwapfsProxyControlTest, Swapfs_ProxyRefreshFailurePreservesSpaceAndCommittedBytes_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t initialSpace = MIN_AVAILABLE_DEVICE_SPACE_BYTES + 8192;
    provider.InitWithAvailableSpace(initialSpace, DEFAULT_SPACE_LIMIT_BYTES, 0);
    provider.OnSwapOutCommitted(1024, 4096);
    provider.proxy_ = new TestStorageManagerProxy([](int64_t &) {
        return -1;
    });

    provider.RefreshAvailableSpaceIfNeeded();

    EXPECT_EQ(provider.availableDeviceSpace_, initialSpace);
    EXPECT_EQ(provider.committedSinceSpaceCheckBytes_, 4096u);
    EXPECT_TRUE(provider.featureEnabled_);
}

} // namespace
