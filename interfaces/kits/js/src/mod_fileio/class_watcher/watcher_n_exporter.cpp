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

#include "watcher_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "securec.h"
#include "watcher_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value WatcherNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    unique_ptr<WatcherEntity> watcherEntity = make_unique<WatcherEntity>();
    if (!NClass::SetEntityFor<WatcherEntity>(env, funcArg.GetThisVar(), move(watcherEntity))) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj stat");
        return nullptr;
    }

    return funcArg.GetThisVar();
}

napi_value WatcherNExporter::StopSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        UniError(EINVAL).ThrowErr(env, "get watcherEntity fail");
        return nullptr;
    }

    watchEntity->fsEventReq_.reset();
    return NVal::CreateUndefined(env).val_;
}

napi_value WatcherNExporter::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        UniError(EINVAL).ThrowErr(env, "get watcherEntity fail");
        return nullptr;
    }

    auto cbExec = [watchEntity](napi_env env) -> UniError {
        watchEntity->fsEventReq_.reset();
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    const string procedureName = "FileIOCreaterWatcher";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}

bool WatcherNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("stop", Stop),
        NVal::DeclareNapiFunction("stopSync", StopSync),
    };

    string className = GetClassName();
    auto [resDefineClass, classValue] = NClass::DefineClass(exports_.env_,
                                                            className,
                                                            WatcherNExporter::Constructor,
                                                            std::move(props));
    if (!resDefineClass) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }

    bool succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string WatcherNExporter::GetClassName()
{
    return WatcherNExporter::className_;
}

WatcherNExporter::WatcherNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

WatcherNExporter::~WatcherNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS