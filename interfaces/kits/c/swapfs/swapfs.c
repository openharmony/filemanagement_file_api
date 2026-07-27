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

#include "swapfs.h"

#define LOG_DOMAIN 0xD004388
#define LOG_TAG "Swapfs"
#include "hilog/log.h"

#include "swapfs_c_api.h"

static void LogNullPointer(const char *function)
{
    (void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, LOG_TAG,
        "%{public}s received a null pointer", function);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_CreateManager(
    const OH_SwapfsConfig *config, OH_SwapfsManager **manager)
{
    if (manager == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    *manager = NULL;
    return SwapfsNativeCreateManager(config, manager);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_DestroyManager(
    OH_SwapfsManager *manager)
{
    if (manager == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeDestroyManager(manager);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_SwapOut(
    OH_SwapfsManager *manager, const OH_SwapfsSwapOutRequest *request, uint64_t *keyId)
{
    if (manager == NULL || request == NULL || keyId == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeSwapOut(manager, request, keyId);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_SwapIn(
    OH_SwapfsManager *manager, const OH_SwapfsSwapInRequest *request, uint64_t *readSize)
{
    if (manager == NULL || request == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeSwapIn(manager, request, readSize);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_QueryData(
    OH_SwapfsManager *manager, uint64_t keyId, OH_SwapfsDataInfo *info)
{
    if (manager == NULL || info == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeQueryData(manager, keyId, info);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_GetStats(
    OH_SwapfsManager *manager, OH_SwapfsStats *stats)
{
    if (manager == NULL || stats == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeGetStats(manager, stats);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_RemoveData(
    OH_SwapfsManager *manager, uint64_t keyId)
{
    if (manager == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeRemoveData(manager, keyId);
}

__attribute__((visibility("default"))) OH_Swapfs_ErrCode OH_Swapfs_RemoveAllData(
    OH_SwapfsManager *manager)
{
    if (manager == NULL) {
        LogNullPointer(__func__);
        return SWAPFS_E_INVAL;
    }
    return SwapfsNativeRemoveAllData(manager);
}
