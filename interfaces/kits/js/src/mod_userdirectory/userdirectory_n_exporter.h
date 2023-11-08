/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef USERDIRECTORY_N_EXPORTER_H
#define USERDIRECTORY_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleUserDirectory {
napi_value GetUserDownloadDir(napi_env env, napi_callback_info info);
napi_value GetUserDesktopDir(napi_env env, napi_callback_info info);
napi_value GetUserDocumentsDir(napi_env env, napi_callback_info info);
napi_value GetUserHomeDir(napi_env env, napi_callback_info info);
napi_value GetExternalStorageDir(napi_env env, napi_callback_info info);
} // namespace ModuleUserDirectory
} // namespace FileManagement
} // namespace OHOS
#endif // USERDIRECTORY_N_EXPORTER_H
