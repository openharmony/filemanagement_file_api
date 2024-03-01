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

#include "environment.h"

#include <stdio.h>

#include "environment_native.h"
#include "../common/error_code.h"

int OH_Environment_GetUserDownloadDir(char **result)
{
    if (result == NULL) {
        return GetErrorCode(-PARAMETER_ERROR);
    }
    int ret = GetUserDir("ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY", result);
    if (ret < 0) {
        return GetErrorCode(ret);
    }
    return ret;
}

int OH_Environment_GetUserDocumentDir(char **result)
{
    if (result == NULL) {
        return GetErrorCode(-PARAMETER_ERROR);
    }
    int ret = GetUserDir("ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY", result);
    if (ret < 0) {
        return GetErrorCode(ret);
    }
    return ret;
}

int OH_Environment_GetUserDesktopDir(char **result)
{
    if (result == NULL) {
        return GetErrorCode(-PARAMETER_ERROR);
    }
    int ret = GetUserDir("ohos.permission.READ_WRITE_DESKTOP_DIRECTORY", result);
    if (ret < 0) {
        return GetErrorCode(ret);
    }
    return ret;
}
