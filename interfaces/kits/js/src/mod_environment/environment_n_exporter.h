/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ENVIRONMENT_N_EXPORTER_H
#define ENVIRONMENT_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
napi_value GetStorageDataDir(napi_env env, napi_callback_info info);
napi_value GetUserDataDir(napi_env env, napi_callback_info info);
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS
#endif // ENVIRONMENT_N_EXPORTER_H