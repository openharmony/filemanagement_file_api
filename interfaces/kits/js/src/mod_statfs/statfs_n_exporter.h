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

#ifndef STATFS_N_EXPORTER_H
#define STATFS_N_EXPORTER_H

#include "filemgmt_libn.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStatfs {
napi_value GetFrSizeSync(napi_env env, napi_callback_info info);
napi_value GetFrSize(napi_env env, napi_callback_info info);

napi_value GetBSizeSync(napi_env env, napi_callback_info info);
napi_value GetBSize(napi_env env, napi_callback_info info);

napi_value GetBAvailSync(napi_env env, napi_callback_info info);
napi_value GetBAvail(napi_env env, napi_callback_info info);

napi_value GetBlocksSync(napi_env env, napi_callback_info info);
napi_value GetBlocks(napi_env env, napi_callback_info info);

napi_value GetBFreeSync(napi_env env, napi_callback_info info);
napi_value GetBFree(napi_env env, napi_callback_info info);

napi_value GetFreeBytesSync(napi_env env, napi_callback_info info);
napi_value GetFreeBytes(napi_env env, napi_callback_info info);

napi_value GetTotalBytesSync(napi_env env, napi_callback_info info);
napi_value GetTotalBytes(napi_env env, napi_callback_info info);
} // namespace ModuleStatfs
} // namespace FileManagement
} // namespace OHOS
#endif // STATFS_N_EXPORTER_H