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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_PROXY_CONTROL_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_PROXY_CONTROL_H

#include <cstdint>
#include <mutex>
#include <string>

#include "istorage_manager.h"
#include "swapfs_control.h"
#include "swapfs.h"

namespace OHOS::FileManagement::Swapfs {

class ProxySwapControlProvider : public SwapControlProvider {
public:
    int Init(uint64_t spaceLimitBytes, uint32_t intervalMs) override;
    int ReserveSwapOut(uint64_t occupiedSize) override;
    void CancelReservedSwapOut(uint64_t occupiedSize) override;
    void OnSwapOutCommitted(uint64_t dataSize, uint64_t occupiedSize) override;
    void OnEntryRemoved(uint64_t dataSize, uint64_t occupiedSize) override;
    void OnAllEntriesRemoved() override;
    void FillStats(OH_SwapfsStats &stats) override;

private:
    void InitWithAvailableSpace(uint64_t availableDeviceSpace, uint64_t spaceLimitBytes, uint32_t intervalMs);
    void RefreshAvailableSpaceIfNeeded();
    int CheckSwapOutAllowedLocked(uint64_t occupiedSize);
    OHOS::sptr<OHOS::StorageManager::IStorageManager> GetOrCreateProxy();
    virtual OHOS::sptr<OHOS::IRemoteObject> GetStorageManagerRemote();
    virtual OHOS::sptr<OHOS::StorageManager::IStorageManager> CastStorageManagerProxy(
        const OHOS::sptr<OHOS::IRemoteObject> &remoteObj);

    OHOS::sptr<OHOS::StorageManager::IStorageManager> proxy_;
    bool proxyUnavailable_ = false;

    std::mutex stateMutex_;
    std::mutex refreshMutex_;
    uint32_t intervalMs_ = DEFAULT_SPACE_CHECK_INTERVAL_MS;
    uint64_t totalKeys_ = 0;
    uint64_t totalDataSize_ = 0;
    uint64_t totalOccupiedSize_ = 0;
    uint64_t pendingOccupiedSize_ = 0;
    uint64_t committedSinceSpaceCheckBytes_ = 0;
    uint64_t spaceLimitBytes_ = DEFAULT_SPACE_LIMIT_BYTES;
    bool featureEnabled_ = true;
    OH_SwapfsDisableReason disableReason_ = OH_SWAPFS_DISABLE_REASON_NONE;
    uint64_t accumulatedWriteBytes_ = 0;
    int64_t lastSpaceCheckTime_ = 0;
    uint64_t availableDeviceSpace_ = 0;
};

} // namespace OHOS::FileManagement::Swapfs

#endif
