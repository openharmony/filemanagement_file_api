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

#include "swapfs_proxy_control.h"

#include <cinttypes>

#include "filemgmt_libhilog.h"

#include <chrono>

#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS::FileManagement::Swapfs {
namespace {
int64_t NowMs()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

constexpr int32_t STORAGE_MANAGER_SA_ID = STORAGE_MANAGER_MANAGER_ID;
} // namespace

OHOS::sptr<OHOS::IRemoteObject> ProxySwapControlProvider::GetStorageManagerRemote()
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("[Swapfs] SystemAbilityManager is null");
        return nullptr;
    }
    auto remoteObj = samgr->GetSystemAbility(STORAGE_MANAGER_SA_ID);
    if (remoteObj == nullptr) {
        HILOGW("[Swapfs] StorageManager SA (5003) not registered");
    }
    return remoteObj;
}

OHOS::sptr<OHOS::StorageManager::IStorageManager> ProxySwapControlProvider::CastStorageManagerProxy(
    const OHOS::sptr<OHOS::IRemoteObject> &remoteObj)
{
    return OHOS::iface_cast<OHOS::StorageManager::IStorageManager>(remoteObj);
}

OHOS::sptr<OHOS::StorageManager::IStorageManager> ProxySwapControlProvider::GetOrCreateProxy()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }

    auto remoteObj = GetStorageManagerRemote();
    if (remoteObj == nullptr) {
        proxyUnavailable_ = true;
        return nullptr;
    }

    proxy_ = CastStorageManagerProxy(remoteObj);
    if (proxy_ == nullptr) {
        HILOGW("[Swapfs] StorageManager iface_cast failed");
        proxyUnavailable_ = true;
        return nullptr;
    }

    HILOGI("[Swapfs] StorageManager proxy created");
    proxyUnavailable_ = false;
    return proxy_;
}

int ProxySwapControlProvider::Init(uint64_t spaceLimitBytes, uint32_t intervalMs)
{
    uint64_t availableDeviceSpace = 0;
    {
        std::lock_guard<std::mutex> lock(refreshMutex_);
        auto proxy = GetOrCreateProxy();
        if (proxy != nullptr) {
            int64_t freeSize = 0;
            int32_t ret = proxy->GetFreeSize(freeSize);
            if (ret == 0) {
                availableDeviceSpace = static_cast<uint64_t>(freeSize);
                HILOGI("[Swapfs] availableDeviceSpace from StorageManager: %{public}" PRIu64, availableDeviceSpace);
            } else {
                HILOGW("[Swapfs] GetFreeSize IPC failed, ret: %{public}d", ret);
            }
        } else {
            HILOGW("[Swapfs] StorageManager SA unavailable");
        }
    }

    InitWithAvailableSpace(availableDeviceSpace, spaceLimitBytes, intervalMs);
    return SWAPFS_E_OK;
}

void ProxySwapControlProvider::InitWithAvailableSpace(uint64_t availableDeviceSpace, uint64_t spaceLimitBytes,
    uint32_t intervalMs)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    totalKeys_ = 0;
    totalDataSize_ = 0;
    totalOccupiedSize_ = 0;
    pendingOccupiedSize_ = 0;
    committedSinceSpaceCheckBytes_ = 0;
    spaceLimitBytes_ = spaceLimitBytes == 0 ? DEFAULT_SPACE_LIMIT_BYTES : spaceLimitBytes;
    featureEnabled_ = true;
    disableReason_ = OH_SWAPFS_DISABLE_REASON_NONE;
    accumulatedWriteBytes_ = 0;
    lastSpaceCheckTime_ = NowMs();
    intervalMs_ = intervalMs;
    availableDeviceSpace_ = availableDeviceSpace;
    if (availableDeviceSpace_ < MIN_AVAILABLE_DEVICE_SPACE_BYTES) {
        featureEnabled_ = false;
        disableReason_ = OH_SWAPFS_DISABLE_REASON_NOSPC;
        HILOGW("[Swapfs] feature disabled, available space below threshold");
    }
}

void ProxySwapControlProvider::RefreshAvailableSpaceIfNeeded()
{
    std::lock_guard<std::mutex> refreshLock(refreshMutex_);
    int64_t now = NowMs();
    uint64_t committedAtStart = 0;
    {
        std::lock_guard<std::mutex> stateLock(stateMutex_);
        if (intervalMs_ > 0 && now - lastSpaceCheckTime_ < static_cast<int64_t>(intervalMs_)) {
            return;
        }
        committedAtStart = committedSinceSpaceCheckBytes_;
    }

    bool refreshed = false;
    uint64_t refreshedSpace = 0;
    if (!proxyUnavailable_) {
        auto proxy = GetOrCreateProxy();
        if (proxy != nullptr) {
            int64_t freeSize = 0;
            int32_t ret = proxy->GetFreeSize(freeSize);
            if (ret == 0) {
                refreshed = true;
                refreshedSpace = static_cast<uint64_t>(freeSize);
                HILOGI("[Swapfs] availableDeviceSpace refreshed from StorageManager: %{public}" PRIu64,
                    refreshedSpace);
            } else {
                HILOGW("[Swapfs] GetFreeSize IPC failed, ret: %{public}d, keeping previous value", ret);
            }
        } else {
            HILOGW("[Swapfs] StorageManager SA unavailable, keeping previous value");
        }
    }

    std::lock_guard<std::mutex> stateLock(stateMutex_);
    if (refreshed) {
        availableDeviceSpace_ = refreshedSpace;
        committedSinceSpaceCheckBytes_ = committedSinceSpaceCheckBytes_ >= committedAtStart ?
            committedSinceSpaceCheckBytes_ - committedAtStart : 0;
    }
    lastSpaceCheckTime_ = now;
    bool wasEnabled = featureEnabled_;
    featureEnabled_ = availableDeviceSpace_ >= MIN_AVAILABLE_DEVICE_SPACE_BYTES;
    disableReason_ = featureEnabled_ ? OH_SWAPFS_DISABLE_REASON_NONE :
        OH_SWAPFS_DISABLE_REASON_NOSPC;
    if (!wasEnabled && featureEnabled_) {
        HILOGI("[Swapfs] feature re-enabled, available space recovered");
    }
}

int ProxySwapControlProvider::CheckSwapOutAllowedLocked(uint64_t occupiedSize)
{
    if (!featureEnabled_) {
        HILOGW("[Swapfs] SwapOut rejected, feature disabled, reason: %{public}d", disableReason_);
        return SWAPFS_E_NOSPC;
    }
    uint64_t usedSize = totalOccupiedSize_ + pendingOccupiedSize_;
    if (usedSize > spaceLimitBytes_ || occupiedSize > spaceLimitBytes_ - usedSize) {
        HILOGW("[Swapfs] SwapOut rejected, quota exceeded, usedSize: %{public}" PRIu64, usedSize);
        return SWAPFS_E_QUOTA_EXCEEDED;
    }
    uint64_t writableSpace = availableDeviceSpace_ - MIN_AVAILABLE_DEVICE_SPACE_BYTES;
    uint64_t inFlightSpace = pendingOccupiedSize_ + committedSinceSpaceCheckBytes_;
    if (inFlightSpace < pendingOccupiedSize_ || inFlightSpace > writableSpace ||
        occupiedSize > writableSpace - inFlightSpace) {
        HILOGW("[Swapfs] SwapOut rejected, device space insufficient");
        return SWAPFS_E_NOSPC;
    }
    return SWAPFS_E_OK;
}

int ProxySwapControlProvider::ReserveSwapOut(uint64_t occupiedSize)
{
    RefreshAvailableSpaceIfNeeded();
    std::lock_guard<std::mutex> lock(stateMutex_);
    int ret = CheckSwapOutAllowedLocked(occupiedSize);
    if (ret != SWAPFS_E_OK) {
        return ret;
    }
    pendingOccupiedSize_ += occupiedSize;
    return SWAPFS_E_OK;
}

void ProxySwapControlProvider::CancelReservedSwapOut(uint64_t occupiedSize)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    pendingOccupiedSize_ = pendingOccupiedSize_ >= occupiedSize ? pendingOccupiedSize_ - occupiedSize : 0;
}

void ProxySwapControlProvider::OnSwapOutCommitted(uint64_t dataSize, uint64_t occupiedSize)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    pendingOccupiedSize_ = pendingOccupiedSize_ >= occupiedSize ? pendingOccupiedSize_ - occupiedSize : 0;
    ++totalKeys_;
    totalDataSize_ += dataSize;
    totalOccupiedSize_ += occupiedSize;
    accumulatedWriteBytes_ += occupiedSize;
    committedSinceSpaceCheckBytes_ += occupiedSize;
}

void ProxySwapControlProvider::OnEntryRemoved(uint64_t dataSize, uint64_t occupiedSize)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (totalKeys_ > 0) {
        --totalKeys_;
    }
    totalDataSize_ = totalDataSize_ >= dataSize ? totalDataSize_ - dataSize : 0;
    totalOccupiedSize_ = totalOccupiedSize_ >= occupiedSize ? totalOccupiedSize_ - occupiedSize : 0;
}

void ProxySwapControlProvider::OnAllEntriesRemoved()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    totalKeys_ = 0;
    totalDataSize_ = 0;
    totalOccupiedSize_ = 0;
}

void ProxySwapControlProvider::FillStats(OH_SwapfsStats &stats)
{
    RefreshAvailableSpaceIfNeeded();
    std::lock_guard<std::mutex> lock(stateMutex_);
    stats.totalKeys = totalKeys_;
    stats.totalDataSize = totalDataSize_;
    stats.totalOccupiedSize = totalOccupiedSize_;
    stats.spaceLimitBytes = spaceLimitBytes_;
    stats.featureEnabled = featureEnabled_;
    stats.disableReason = disableReason_;
    stats.accumulatedWriteBytes = accumulatedWriteBytes_;
    stats.lastSpaceCheckTime = lastSpaceCheckTime_;
    stats.availableDeviceSpace = availableDeviceSpace_;
}
} // namespace OHOS::FileManagement::Swapfs
