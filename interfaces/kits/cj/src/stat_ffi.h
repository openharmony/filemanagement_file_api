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

#ifndef OHOS_FILE_FS_STAT_FFI_H
#define OHOS_FILE_FS_STAT_FFI_H

#include "ffi_remote_data.h"
#include "cj_common_ffi.h"

#include <cstdint>

extern "C" {
    int64_t FfiOHOSStatGetIno(int64_t id);
    int64_t FfiOHOSStatGetMode(int64_t id);
    int64_t FfiOHOSStatGetUid(int64_t id);
    int64_t FfiOHOSStatGetGid(int64_t id);
    int64_t FfiOHOSStatGetSize(int64_t id);
    int64_t FfiOHOSStatGetAtime(int64_t id);
    int64_t FfiOHOSStatGetMtime(int64_t id);
    int64_t FfiOHOSStatGetCtime(int64_t id);
    bool FfiOHOSStatIsBlockDevice(int64_t id);
    bool FfiOHOSStatIsCharacterDevice(int64_t id);
    bool FfiOHOSStatIsDirectory(int64_t id);
    bool FfiOHOSStatIsFIFO(int64_t id);
    bool FfiOHOSStatIsFile(int64_t id);
    bool FfiOHOSStatIsSocket(int64_t id);
    bool FfiOHOSStatIsSymbolicLink(int64_t id);
}

#endif // OHOS_FILE_FS_STAT_FFI_H