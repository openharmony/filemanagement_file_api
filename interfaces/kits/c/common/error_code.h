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

enum ErrorCode {
    PERMISSION_ERROR = 201,
    PARAMETER_ERROR = 401,
    DEVICE_NOT_SUPPORTED = 801,
    E_PERM = 13900001,
    E_NOENT = 13900002,
    E_NOMEM = 13900011,
    UNKNOWN_ERROR = 13900042
};

typedef struct {
    int errNo;
    int errorCode;
} Map;

static Map g_map[] = {
    { UNKNOWN_ERROR, UNKNOWN_ERROR },
    { -PERMISSION_ERROR, PERMISSION_ERROR },
    { -PARAMETER_ERROR, PARAMETER_ERROR },
    { -DEVICE_NOT_SUPPORTED, DEVICE_NOT_SUPPORTED },
    { -EPERM, E_PERM },
    { -ENOENT, E_NOENT },
    { -ENOMEM, E_NOMEM },
};

static int GetErrorCode(int errNum)
{
    for (size_t i = 1; i < sizeof(g_map) / sizeof(g_map[0]); i++) {
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
