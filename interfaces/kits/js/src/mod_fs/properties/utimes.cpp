/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "utimes.h"

#include <unistd.h>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value Utimes::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succGetPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succGetPath) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succGetMtime, mtime] = NVal(env, funcArg[NARG_POS::SECOND]).ToDouble();;
    if (!succGetMtime || mtime < 0) {
        HILOGE("Invalid mtime from JS second argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    int ret = uv_fs_stat(nullptr, stat_req.get(), path.get(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of the file by path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> utimes_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!utimes_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    double atime = static_cast<double>(stat_req->statbuf.st_atim.tv_sec) +
        static_cast<double>(stat_req->statbuf.st_atim.tv_nsec) / NS;
    ret = uv_fs_utime(nullptr, utimes_req.get(), path.get(), atime, mtime / MS, nullptr);
    if (ret < 0) {
        HILOGE("Failed to chang mtime of the file for %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

} // ModuleFileIO
} // FileManagement
} // OHOS