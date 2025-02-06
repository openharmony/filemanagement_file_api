/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILE_FFI_H
#define OHOS_FILE_FFI_H

#include "cj_common_ffi.h"
#include <cstdint>
#include "napi/native_api.h"

extern "C" {
    FFI_EXPORT napi_value FfiConvertFile2Napi(napi_env env, int64_t id);
    FFI_EXPORT int64_t FfiCreateFileFromNapi(napi_env env, napi_value objRAF);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsOpen(const char* path, int64_t mode);
    FFI_EXPORT int32_t FfiOHOSFileFsClose(int64_t file);
    FFI_EXPORT int32_t FfiOHOSFileFsCloseByFd(int32_t file);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsDup(int32_t fd);
    FFI_EXPORT int32_t FfiOHOSFILEFsGetFD(int64_t id);
    FFI_EXPORT const char* FfiOHOSFILEFsGetPath(int64_t id);
    FFI_EXPORT const char* FfiOHOSFILEFsGetName(int64_t id);
    FFI_EXPORT RetCode FfiOHOSFILEFsTryLock(int64_t id, bool exclusive);
    FFI_EXPORT RetCode FfiOHOSFILEFsUnLock(int64_t id);
    FFI_EXPORT RetDataCString FfiOHOSFILEFsGetParent(int64_t id);
}

#endif // OHOS_FILE_FFI_H
