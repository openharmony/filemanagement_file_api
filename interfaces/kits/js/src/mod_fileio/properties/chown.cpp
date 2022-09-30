/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "chown.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, string, int, int> GetChownArg(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return { false, "", -1, -1 };
    }

    auto [resGetSecondArg, owner] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!resGetSecondArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid owner");
        return { false, "", -1, -1 };
    }

    auto [resGetThirdArg, group] = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
    if (!resGetThirdArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid group");
        return { false, "", -1, -1 };
    }

    return { true, path.get(), owner, group };
}

napi_value Chown::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetChownArg, path, owner, group] = GetChownArg(env, funcArg);
    if (!resGetChownArg) {
        return nullptr;
    }

    if (chown(path.c_str(), owner, group) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Chown::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetChownArg, path, owner, group] = GetChownArg(env, funcArg);
    if (!resGetChownArg) {
        return nullptr;
    }

    auto cbExec = [path = path, owner = owner, group = group](napi_env env) -> UniError {
        if (chown(path.c_str(), owner, group) == -1) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    const string procedureName = "FileIOChown";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = NARG_POS::FOURTH;
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
