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


#ifndef OHOS_STREAM_FFI_H
#define OHOS_STREAM_FFI_H

#include "macro.h"
#include "cj_common_ffi.h"
#include "stream_impl.h"

extern "C" {
FFI_EXPORT RetCode FfiOHOSStreamClose(int64_t id);
FFI_EXPORT RetCode FfiOHOSStreamFlush(int64_t id);
FFI_EXPORT RetDataI64 FfiOHOSStreamWriteCur(int64_t id, const char* buffer, int64_t length, const char* encode);
FFI_EXPORT RetDataI64 FfiOHOSStreamWrite(int64_t id,
    const char* buffer, int64_t length, int64_t offset, const char* encode);
FFI_EXPORT RetDataI64 FfiOHOSStreamReadCur(int64_t id, uint8_t* buffer, int64_t bufLen, int64_t length);
FFI_EXPORT RetDataI64 FfiOHOSStreamRead(int64_t id, uint8_t* buffer, int64_t bufLen, int64_t length, int64_t offset);
}

#endif // OHOS_STREAM_FFI_H
