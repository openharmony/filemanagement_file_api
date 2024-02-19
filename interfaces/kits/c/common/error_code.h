/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_C_COMMON_ERROR_CODE_H
#define OHOS_FILEMANAGEMENT_FILE_API_C_COMMON_ERROR_CODE_H

#include <errno.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */
#define MAP_SIZE 6
#define UNKNOWN_ERROR 13900042
#define PERMISSION_ERROR 201
#define DEVICE_NOT_SUPPORTED 801
#define FILEIO_SYS_CAP_ID 13900000

typedef struct {
    int errNo;
    int errorCode;
} Map;

static Map g_map[MAP_SIZE] = {
    { UNKNOWN_ERROR, UNKNOWN_ERROR },
    { -PERMISSION_ERROR, PERMISSION_ERROR },
    { -DEVICE_NOT_SUPPORTED, DEVICE_NOT_SUPPORTED },
    { -EPERM, FILEIO_SYS_CAP_ID + EPERM },
    { -ENOENT, FILEIO_SYS_CAP_ID + ENOENT },
    { -ENOMEM, FILEIO_SYS_CAP_ID + ENOMEM - 1 },
};

static int GetErrorCode(int errNum)
{
    for (int i = 1; i < MAP_SIZE; i++) {
        if (g_map[i].errNo == errNum) {
            return g_map[i].errorCode;
        }
    }
    return g_map[0].errorCode;
}
#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif /* OHOS_FILEMANAGEMENT_FILE_API_C_COMMON_ERROR_CODE_H */
