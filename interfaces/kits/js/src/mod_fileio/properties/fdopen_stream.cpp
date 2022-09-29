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

#include "fdopen_stream.h"
#include <memory>
#include <tuple>

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"
#include "../class_stream/stream_entity.h"
#include "../class_stream/stream_n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static NVal InstantiateStream(napi_env env, unique_ptr<FILE, decltype(&fclose)> fp)
{
    napi_value objStream = NClass::InstantiateClass(env, StreamNExporter::className_, {});
    if (!objStream) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Cannot instantiate stream");
        return NVal();
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, objStream);
    if (!streamEntity) {
        UniError(EIO).ThrowErr(env, "Cannot instantiate stream because of void entity");
        return NVal();
    }

    streamEntity->fp.swap(fp);
    return { env, objStream };
}

static tuple<bool, int, string> GetFdopenStreamArgs(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Arg fd is required to be type integer");
        return { false, -1, "" };
    }

    auto [resGetSecondArg, mode, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        UniError(EINVAL).ThrowErr(env, "Arg mode is required to be type string");
        return { false, -1, "" };
    }

    return { true, fd, mode.get() };
}

napi_value FdopenStream::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFdopenStreamArgs, fd, mode] = GetFdopenStreamArgs(env, funcArg);
    if (!resGetFdopenStreamArgs) {
        return nullptr;
    }

    unique_ptr<FILE, decltype(&fclose)> fp = { fdopen(fd, mode.c_str()), fclose };
    if (!fp) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return InstantiateStream(env, move(fp)).val_;
}

struct AsyncFdopenStreamArg {
    unique_ptr<FILE, decltype(&fclose)> fp = { nullptr, fclose };
};

napi_value FdopenStream::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFdopenStreamArgs, fd, mode] = GetFdopenStreamArgs(env, funcArg);
    if (!resGetFdopenStreamArgs) {
        return nullptr;
    }

    auto arg = make_shared<AsyncFdopenStreamArg>();
    auto cbExec = [arg = arg, fd = fd, mode = mode](napi_env env) -> UniError {
        arg->fp = { fdopen(fd, mode.c_str()), fclose };
        if (!arg->fp) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg = arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateStream(env, move(arg->fp));
    };

    const string procedureName = "FileIOFdopenStream";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS