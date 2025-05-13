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

#include "cj_common_ffi.h"
#include "macro.h"
#include "uni_error.h"
#include "xattr.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {
extern "C" {

FFI_EXPORT void FfiOHOSFileFsReleaseCString(char *str)
{
    LOGD("FS_TEST::FfiOHOSFileFsReleaseCString");
    free(str);
}

FFI_EXPORT int32_t FfiOHOSFileFsSetXattr(const char *path, const char *key, const char *value)
{
    LOGD("FS_TEST::FfiOHOSFileFsSetXattr");
    if (path == nullptr || key == nullptr || value == nullptr) {
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto state = Xattr::SetSync(path, key, value);
    if (state != SUCCESS_CODE) {
        LOGE("FS_TEST::FfiOHOSFileFsSetXattr error");
        return GetErrorCode(state);
    }
    return SUCCESS_CODE;
}

FFI_EXPORT RetDataCString FfiOHOSFileFsGetXattr(const char *path, const char *key)
{
    LOGD("FS_TEST::FfiOHOSFileFsGetXattr");
    RetDataCString retData = { .code = ERR_INVALID_INSTANCE_CODE, .data = nullptr };
    if (path == nullptr || key == nullptr) {
        return retData;
    }
    auto [state, result] = Xattr::GetSync(path, key);
    if (state != SUCCESS_CODE) {
        LOGE("FS_TEST::FfiOHOSFileFsGetXattr error");
        retData.code = GetErrorCode(state);
        return retData;
    }
    retData.data = result;
    retData.code = SUCCESS_CODE;
    return retData;
}
}
}  // namespace FileFs
}  // namespace CJSystemapi
}  // namespace OHOS
