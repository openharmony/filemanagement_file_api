/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

extern "C" {
    RetDataI64 FfiOHOSFileFsOpen(const char* path, int64_t mode);
    int32_t FfiOHOSFileFsClose(int64_t file);
    int32_t FfiOHOSFileFsCloseByFd(int32_t file);
    RetDataI64 FfiOHOSFileFsDup(int32_t fd);
    int32_t FfiOHOSFileFsGetFD(int64_t id);
    const char* FfiOHOSFILEFsGetPath(int64_t id);
    const char* FfiOHOSFILEFsGetName(int64_t id);
    RetCode FfiOHOSFILEFsTryLock(int64_t id, bool exclusive);
    RetCode FfiOHOSFILEFsUnLock(int64_t id);
    RetDataCString FfiOHOSFILEFsGetParent(int64_t id);
}

#endif // OHOS_FILE_FFI_H
