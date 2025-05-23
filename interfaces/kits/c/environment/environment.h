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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_C_ENVIRONMENT_ENVIRONMENT_H
#define OHOS_FILEMANAGEMENT_FILE_API_C_ENVIRONMENT_ENVIRONMENT_H

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */
int OH_Environment_GetUserDownloadDir(char **result);
int OH_Environment_GetUserDesktopDir(char **result);
int OH_Environment_GetUserDocumentDir(char **result);
#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif /* OHOS_FILEMANAGEMENT_FILE_API_C_ENVIRONMENT_ENVIRONMENT_H */