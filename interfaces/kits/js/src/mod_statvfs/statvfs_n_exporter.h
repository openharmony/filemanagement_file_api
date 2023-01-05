/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef STATVFS_N_EXPORTER_H
#define STATVFS_N_EXPORTER_H

#include "filemgmt_libn.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStatvfs {
napi_value GetFreeSizeSync(napi_env env, napi_callback_info info);
napi_value GetFreeSize(napi_env env, napi_callback_info info);

napi_value GetTotalSizeSync(napi_env env, napi_callback_info info);
napi_value GetTotalSize(napi_env env, napi_callback_info info);
} // namespace ModuleStatvfs
} // namespace FileManagement
} // namespace OHOS
#endif // STATVFS_N_EXPORTER_H