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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_CONTROL_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_CONTROL_H

#include <cstdint>
#include "swapfs.h"

namespace OHOS::FileManagement::Swapfs {
constexpr uint64_t DEFAULT_SPACE_LIMIT_BYTES = 1024ULL * 1024ULL * 1024ULL;
constexpr uint64_t MIN_AVAILABLE_DEVICE_SPACE_BYTES = 5ULL * 1024ULL * 1024ULL * 1024ULL;
constexpr uint32_t DEFAULT_SPACE_CHECK_INTERVAL_MS = 1000;

class SwapControlProvider {
public:
    virtual ~SwapControlProvider() = default;
    virtual int Init(uint64_t spaceLimitBytes, uint32_t intervalMs) = 0;
    virtual int ReserveSwapOut(uint64_t occupiedSize) = 0;
    virtual void CancelReservedSwapOut(uint64_t occupiedSize) = 0;
    virtual void OnSwapOutCommitted(uint64_t dataSize, uint64_t occupiedSize) = 0;
    virtual void OnEntryRemoved(uint64_t dataSize, uint64_t occupiedSize) = 0;
    virtual void OnAllEntriesRemoved() = 0;
    virtual void FillStats(OH_SwapfsStats &stats) = 0;
};
} // namespace OHOS::FileManagement::Swapfs

#endif
