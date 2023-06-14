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

#include "rmdirent.h"

#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

#ifdef __MUSL__
static NError RmDirent(const string &fpath)
{
    std::filesystem::path strToPath(fpath);
    std::error_code errCode;
    std::uintmax_t num = std::filesystem::remove_all(strToPath, errCode);
    if (errCode) {
        HILOGE("Failed to remove directory, error code: %{public}d", errCode.value());
        return NError(errCode.value());
    }
    if (!num || std::filesystem::exists(strToPath)) {
        HILOGE("Failed to remove directory, dirPath does not exist");
        return NError(ENOENT);
    }
    return NError(ERRNO_NOERR);
}

#else
static NError RmDirent(const string &fpath)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> scandir_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!scandir_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = 0;
    ret = uv_fs_scandir(nullptr, scandir_req.get(), fpath.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to scandir, ret: %{public}d", ret);
        return NError(ret);
    }
    uv_dirent_t dent;
    while (uv_fs_scandir_next(scandir_req.get(), &dent) != UV_EOF) {
        string filePath = fpath + "/" + string(dent.name);
        if (dent.type == UV_DIRENT_FILE) {
            std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> unlink_req = {
                new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
            if (!unlink_req) {
                HILOGE("Failed to request heap memory.");
                return NError(ENOMEM);
            }
            ret = uv_fs_unlink(nullptr, unlink_req.get(), filePath.c_str(), nullptr);
            if (ret < 0) {
                HILOGE("Failed to unlink file, ret: %{public}d", ret);
                return NError(ret);
            }
        } else if (dent.type == UV_DIRENT_DIR) {
            auto rmDirentRes = RmDirent(filePath);
            if (rmDirentRes) {
                return rmDirentRes;
            }
        }
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> rmdir_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!rmdir_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    ret = uv_fs_rmdir(nullptr, rmdir_req.get(), fpath.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to rmdir empty dir, ret: %{public}d", ret);
        return NError(ret);
    }
    return NError(ERRNO_NOERR);
}
#endif

napi_value Rmdirent::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto err = RmDirent(string(path.get()));
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Rmdirent::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [tmpPath = string(path.get())]() -> NError {
        return RmDirent(tmpPath);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_RMDIRENT_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_RMDIRENT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS