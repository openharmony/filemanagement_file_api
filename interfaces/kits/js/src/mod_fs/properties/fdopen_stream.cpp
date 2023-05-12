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

#include "fdopen_stream.h"

#include <memory>
#include <tuple>

#include "class_stream/stream_entity.h"
#include "class_stream/stream_n_exporter.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, int, string> GetFdopenStreamArgs(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!resGetFirstArg) {
        return { false, -1, "" };
    }

    auto [resGetSecondArg, mode, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        return { false, -1, "" };
    }

    return { true, fd, mode.get() };
}

napi_value FdopenStream::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFdopenStreamArgs, fd, mode] = GetFdopenStreamArgs(env, funcArg);
    if (!resGetFdopenStreamArgs || fd < 0) {
        HILOGE("Invalid fd or mode from JS arugments");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<FILE, decltype(&fclose)> fp = { fdopen(fd, mode.c_str()), fclose };
    if (!fp) {
        HILOGE("Failed to fdopen file by fd:%{public}d", fd);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return CommonFunc::InstantiateStream(env, move(fp)).val_;
}

napi_value FdopenStream::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFdopenStreamArgs, fd, mode] = GetFdopenStreamArgs(env, funcArg);
    if (!resGetFdopenStreamArgs || fd < 0) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    shared_ptr<AsyncFdopenStreamArg> arg;
    try {
        arg = make_shared<AsyncFdopenStreamArg>();
    } catch (const bad_alloc &) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, fd = fd, mode = mode]() -> NError {
        arg->fp = { fdopen(fd, mode.c_str()), fclose };
        if (!arg->fp) {
            HILOGE("Failed to fdopen file by fd:%{public}d", fd);
            return NError(errno);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return CommonFunc::InstantiateStream(env, move(arg->fp));
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_FDOPENSTREAM_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_FDOPENSTREAM_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS