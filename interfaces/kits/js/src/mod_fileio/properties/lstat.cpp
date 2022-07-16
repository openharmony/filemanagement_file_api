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
#include "lstat.h"

#include <memory>
#include <tuple>

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"

#include "../class_stat/stat_entity.h"
#include "../class_stat/stat_n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value Lstat::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, pathPtr, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "The first argument requires type string");
        return nullptr;
    }

    struct stat buf;
    int ret = lstat(pathPtr.get(), &buf);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    napi_value objStat = NClass::InstantiateClass(env, StatNExporter::className_, {});
    if (!objStat) {
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, objStat);
    if (!statEntity) {
        return nullptr;
    }

    statEntity->stat_ = buf;
    return objStat;
}

struct AsyncStatArg {
    struct stat stat_;
};

napi_value Lstat::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    string path = tmp.get();
    auto arg = make_shared<AsyncStatArg>();
    auto cbExec = [arg, path](napi_env env) -> UniError {
        if (lstat(path.c_str(), &arg->stat_)) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value objStat = NClass::InstantiateClass(env, StatNExporter::className_, {});
        if (!objStat) {
            return { env, UniError(EIO).GetNapiErr(env) };
        }
        auto statEntity = NClass::GetEntityOf<StatEntity>(env, objStat);
        if (!statEntity) {
            return { env, UniError(EIO).GetNapiErr(env) };
        }
        statEntity->stat_ = arg->stat_;
        return { env, objStat };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    const string procedureName = "FileIOLstat";
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS