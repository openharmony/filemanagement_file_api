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

#include "symlink.h"

#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, string, string> GetSymlinkArg(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, src, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        return { false, "", "" };
    }

    auto [resGetSecondArg, dest, useless] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        return { false, "", "" };
    }

    return { true, src.get(), dest.get() };
}

napi_value Symlink::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetSymlinkArg, oldPath, newPath] = GetSymlinkArg(env, funcArg);
    if (!resGetSymlinkArg) {
        HILOGE("Failed to get symlink arguments");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> symlink_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!symlink_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_symlink(nullptr, symlink_req.get(), oldPath.c_str(), newPath.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a link for old path");
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Symlink::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetSymlinkArg, oldPath, newPath] = GetSymlinkArg(env, funcArg);
    if (!resGetSymlinkArg) {
        HILOGE("Failed to get symlink arguments");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [oldPath = move(oldPath), newPath = move(newPath)]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> symlink_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!symlink_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_symlink(nullptr, symlink_req.get(), oldPath.c_str(), newPath.c_str(), 0, nullptr);
        if (ret < 0) {
            HILOGE("Failed to create a link for old path");
            return NError(errno);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_SYMLINK_NAME, cbExec, cbComplCallback).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_SYMLINK_NAME, cbExec, cbComplCallback).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
