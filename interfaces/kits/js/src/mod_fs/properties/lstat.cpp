/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "lstat.h"

#include <memory>
#include <securec.h>
#include <tuple>

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
const std::string SCHEME_FILE = "file";

static tuple<bool, string> ParsePath(napi_env env, const string &pathStr)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (pathStr.find("://") != string::npos) {
        Uri uri(pathStr);
        string uriType = uri.GetScheme();
        if (uriType == SCHEME_FILE) {
            AppFileService::ModuleFileUri::FileUri fileUri(pathStr);
            string realPath = fileUri.GetRealPath();
            return { true, realPath };
        }
        HILOGE("Failed to lstat file by invalid uri");
        NError(EINVAL).ThrowErr(env);
        return { false, "" };
    }
#endif
    return { true, pathStr };
}

napi_value Lstat::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, pathPtr, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    string pathStr(pathPtr.get());
    auto [succ, path] = ParsePath(env, pathStr);
    if (!succ) {
        return nullptr;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> lstat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!lstat_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_lstat(nullptr, lstat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of file, ret: %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    auto stat = CommonFunc::InstantiateStat(env, lstat_req->statbuf).val_;
    return stat;
}

static NError LstatExec(shared_ptr<StatEntity> arg, const string &path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> lstat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!lstat_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_lstat(nullptr, lstat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of file, ret: %{public}d", ret);
        return NError(ret);
    } else {
        arg->stat_ = lstat_req->statbuf;
        return NError(ERRNO_NOERR);
    }
}

napi_value Lstat::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    string pathStr(tmp.get());
    bool succ;
    std::string path;
    std::tie(succ, path) = ParsePath(env, pathStr);
    if (!succ) {
        return nullptr;
    }
    auto arg = CreateSharedPtr<StatEntity>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, path]() -> NError {
        return LstatExec(arg, path);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        auto stat = CommonFunc::InstantiateStat(env, arg->stat_, true);
        return stat;
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_LSTAT_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_LSTAT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO