/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ftruncate.h"

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

napi_value Ftruncate::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int ret = -1;
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        ret = ftruncate(fd, 0);
    } else {
        int len;
        bool succ = false;
        tie(succ, len) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid len");
            return nullptr;
        }
        ret = ftruncate(fd, len);
    }

    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Ftruncate::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int len = 0;
    size_t argc = funcArg.GetArgc();
    if (argc > NARG_CNT::ONE) {
        auto [resGetSecondArg, length] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!resGetSecondArg) {
            UniError(EINVAL).ThrowErr(env, "Invalid len");
            return nullptr;
        }
        len = length;
    }

    auto cbExec = [fd = fd, len = len](napi_env env) -> UniError {
        int ret = ftruncate(fd, len);
        if (ret == -1) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbComplete = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    const string procedureName = "fileIOFtruncate";
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE || (argc == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS