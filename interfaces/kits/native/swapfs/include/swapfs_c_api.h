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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_C_API_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_C_API_H

#include "swapfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SWAPFS_NATIVE_API
#define SWAPFS_NATIVE_API __attribute__((visibility("default")))
#endif

SWAPFS_NATIVE_API int SwapfsNativeCreateManager(const OH_SwapfsConfig *config, OH_SwapfsManager **manager);
SWAPFS_NATIVE_API int SwapfsNativeDestroyManager(OH_SwapfsManager *manager);
SWAPFS_NATIVE_API int SwapfsNativeSwapOut(
    OH_SwapfsManager *manager, const OH_SwapfsSwapOutRequest *request, uint64_t *keyId);
SWAPFS_NATIVE_API int SwapfsNativeSwapIn(
    OH_SwapfsManager *manager, const OH_SwapfsSwapInRequest *request, uint64_t *readSize);
SWAPFS_NATIVE_API int SwapfsNativeQueryData(OH_SwapfsManager *manager, uint64_t keyId, OH_SwapfsDataInfo *info);
SWAPFS_NATIVE_API int SwapfsNativeGetStats(OH_SwapfsManager *manager, OH_SwapfsStats *stats);
SWAPFS_NATIVE_API int SwapfsNativeRemoveData(OH_SwapfsManager *manager, uint64_t keyId);
SWAPFS_NATIVE_API int SwapfsNativeRemoveAllData(OH_SwapfsManager *manager);

#ifdef __cplusplus
}
#endif
#endif
