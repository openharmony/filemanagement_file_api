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

#include "environment_n_exporter.h"

#include <string>

#include "../common/napi/n_async/n_async_work_callback.h"
#include "../common/napi/n_async/n_async_work_promise.h"
#include "../common/napi/n_class.h"
#include "../common/napi/n_func_arg.h"
#include "../common/napi/n_val.h"
#include "../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleEnvironment {
namespace {
    const std::string STORAGE_DATA_PATH = "/data";
}
napi_value GetStorageDataDir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto cbExec = [](napi_env env) -> UniError {
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, STORAGE_DATA_PATH);
    };

    static const std::string procedureName = "GetStorageDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
}

int GetUserId()
{
    return 0;
}

napi_value GetUserDataDir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto userDataPath = std::make_shared<std::string>();
    auto cbExec = [userDataPath](napi_env env) -> UniError {
        (*userDataPath).append("/storage/media/").append(std::to_string(GetUserId())).append("/local");
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [userDataPath](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, *userDataPath);
    };

    static const std::string procedureName = "GetUserDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
}
} // namespace ModuleEnvironment
} // namespace DistributedFS
} // namespace OHOS