/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file_fs_impl.h"
#include "n_error.h"
#include "mkdtemp.h"
#include "utils.h"

#include <cstring>

using namespace OHOS::FFI;
using namespace OHOS::CJSystemapi;
using namespace OHOS::FileManagement::LibN;


namespace OHOS {
namespace CJSystemapi {

char* MallocCString(const std::string& origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char* res = static_cast<char*>(malloc(sizeof(char) * len));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

RetDataCString MkdtempImpl::Mkdtemp(const std::string& prefix)
{
    LOGI("FS_TEST:: MkdtempImpl::Mkdtemp start");
    RetDataCString ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = nullptr };
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> mkdtemp_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!mkdtemp_req) {
        LOGE("Failed to request heap memory.");
        ret.code = ENOMEM;
        return ret;
    }
    int code = uv_fs_mkdtemp(nullptr, mkdtemp_req.get(), const_cast<char *>(prefix.c_str()), nullptr);
    ret.code = code;
    if (code < 0) {
        LOGE("Failed to create a temporary directory with path");
        return ret;
    }
    ret.data = MallocCString(mkdtemp_req->path);
    LOGI("FS_TEST:: MkdtempImpl::Mkdtemp end");
    return ret;
}

}
} // namespace OHOS::CJSystemapi