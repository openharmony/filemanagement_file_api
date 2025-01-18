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

#include "statvfs_ffi.h"
#include <cinttypes>
#include <sys/statvfs.h>
#include "macro.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace Statvfs {
extern "C" {
RetDataI64 FfiOHOSStatvfsGetFreeSize(char* path)
{
    LOGI("OHOS::CJSystemapi::FfiOHOSStatvfsGetFreeSize");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, size] = StatvfsImpl::GetFreeSize(path);
    ret.code = state;
    ret.data = size;
    LOGI("OHOS::CJSystemapi::FfiOHOSStatvfsGetFreeSize success");
    return ret;
}

RetDataI64 FfiOHOSStatvfsGetTotalSize(char* path)
{
    LOGI("OHOS::CJSystemapi::FfiOHOSStatvfsGetTotalSize");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, size] = StatvfsImpl::GetTotalSize(path);
    ret.code = state;
    ret.data = size;
    LOGI("OHOS::CJSystemapi::FfiOHOSStatvfsGetTotalSize success");
    return ret;
}
}
} // namespace Statvfs
} // namespace CJSystemapi
} // namespace OHOS