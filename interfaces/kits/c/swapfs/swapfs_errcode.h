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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_C_SWAPFS_ERRCODE_H
#define OHOS_FILEMANAGEMENT_FILE_API_C_SWAPFS_ERRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OH_Swapfs_ErrCode {
    SWAPFS_E_OK = 0,
    SWAPFS_E_INVAL = 36200001,
    SWAPFS_E_DIO_ALIGN = 36200002,
    SWAPFS_E_BUFFER_TOO_SMALL = 36200003,
    SWAPFS_E_KEY_NOT_FOUND = 36200004,
    SWAPFS_E_KEY_STATE_INVALID = 36200005,
    SWAPFS_E_BUSY = 36200006,
    SWAPFS_E_NOSPC = 36200007,
    SWAPFS_E_QUOTA_EXCEEDED = 36200008,
    SWAPFS_E_IO_ERROR = 36200009,
    SWAPFS_E_FEATURE_DISABLED = 36200010,
    SWAPFS_E_ACCES = 36200011,
    SWAPFS_E_PATH_UNAVAILABLE = 36200012,
    SWAPFS_E_SHUTTING_DOWN = 36200013,
    SWAPFS_E_NOMEM = 36200014,
} OH_Swapfs_ErrCode;

#ifdef __cplusplus
}
#endif
#endif
