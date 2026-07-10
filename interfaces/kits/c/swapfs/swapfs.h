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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_C_SWAPFS_H
#define OHOS_FILEMANAGEMENT_FILE_API_C_SWAPFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "swapfs_errcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWAPFS_DIO_ALIGNMENT 4096U

typedef enum OH_SwapfsKeyStatus {
    OH_SWAPFS_KEY_STATUS_ACTIVE = 0,
    OH_SWAPFS_KEY_STATUS_REMOVING = 1,
} OH_SwapfsKeyStatus;

typedef enum OH_SwapfsDisableReason {
    OH_SWAPFS_DISABLE_REASON_NONE = 0,
    OH_SWAPFS_DISABLE_REASON_NOSPC = 1,
} OH_SwapfsDisableReason;

typedef struct OH_SwapfsManager OH_SwapfsManager;

typedef struct OH_SwapfsConfig {
    /* Base path. Swapfs stores data in its fixed "swapfs" child directory. */
    const char *swapRootPath;
    uint64_t spaceLimitBytes;
    bool useDirectIo;
} OH_SwapfsConfig;

typedef struct OH_SwapfsSwapOutRequest {
    const void *buffer;
    uint64_t bufferSize;
} OH_SwapfsSwapOutRequest;

typedef struct OH_SwapfsSwapInRequest {
    uint64_t keyId;
    void *buffer;
    uint64_t bufferSize;
} OH_SwapfsSwapInRequest;

typedef struct OH_SwapfsDataInfo {
    uint64_t keyId;
    uint64_t dataSize;
    uint64_t occupiedSize;
    int64_t createTime;
    OH_SwapfsKeyStatus status;
    bool canSwapIn;
} OH_SwapfsDataInfo;

typedef struct OH_SwapfsStats {
    uint64_t totalKeys;
    uint64_t totalDataSize;
    uint64_t totalOccupiedSize;
    uint64_t spaceLimitBytes;
    bool featureEnabled;
    OH_SwapfsDisableReason disableReason;
    uint64_t accumulatedWriteBytes;
    int64_t lastSpaceCheckTime;
    uint64_t availableDeviceSpace;
} OH_SwapfsStats;

OH_Swapfs_ErrCode OH_Swapfs_CreateManager(
    const OH_SwapfsConfig *config, OH_SwapfsManager **manager);
OH_Swapfs_ErrCode OH_Swapfs_DestroyManager(OH_SwapfsManager *manager);
OH_Swapfs_ErrCode OH_Swapfs_SwapOut(
    OH_SwapfsManager *manager, const OH_SwapfsSwapOutRequest *request, uint64_t *keyId);
OH_Swapfs_ErrCode OH_Swapfs_SwapIn(
    OH_SwapfsManager *manager, const OH_SwapfsSwapInRequest *request, uint64_t *readSize);
OH_Swapfs_ErrCode OH_Swapfs_QueryData(
    OH_SwapfsManager *manager, uint64_t keyId, OH_SwapfsDataInfo *info);
OH_Swapfs_ErrCode OH_Swapfs_GetStats(OH_SwapfsManager *manager, OH_SwapfsStats *stats);
OH_Swapfs_ErrCode OH_Swapfs_RemoveData(OH_SwapfsManager *manager, uint64_t keyId);
OH_Swapfs_ErrCode OH_Swapfs_RemoveAllData(OH_SwapfsManager *manager);

#ifdef __cplusplus
}
#endif
#endif
