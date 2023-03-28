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

#include "stream_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <memory>
#include <securec.h>
#include <sstream>
#include <string>

#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "flush.h"
#include "stream_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value StreamNExporter::ReadSync(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may have been closed");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    FILE *filp = nullptr;
    filp = streamEntity->fp.get();

    auto [succ, buf, len, hasOffset, offset] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    int ret = fseek(filp, offset, SEEK_SET);
    if (hasOffset && (ret < 0)) {
        HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    size_t actLen = fread(buf, 1, len, filp);
    if ((actLen != static_cast<size_t>(len) && !feof(filp)) || ferror(filp)) {
        HILOGE("Invalid buffer size and pointer, actlen: %{public}zu", actLen);
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, actLen).val_;
}

napi_value StreamNExporter::CloseSync(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may have been closed yet");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    streamEntity->fp.reset();
    (void)NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());

    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::WriteSync(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may has been closed");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    FILE *filp = nullptr;
    filp = streamEntity->fp.get();

    auto [succ, bufGuard, buf, len, hasOffset, offset] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }
    int ret = fseek(filp, static_cast<long>(offset), SEEK_SET);
    if (hasOffset && (ret < 0)) {
        HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    size_t writeLen = fwrite(buf, 1, len, filp);
    if ((writeLen == 0) && (writeLen != len)) {
        HILOGE("Failed to fwrite stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(writeLen)).val_;
}

static bool HasOption(napi_env env, napi_value optionFromJsArg)
{
    NVal op = NVal(env, optionFromJsArg);
    if (op.HasProp("offset") || op.HasProp("length") || op.HasProp("encoding") || !op.TypeIs(napi_function)) {
        return true;
    }
    return false;
}

napi_value StreamNExporter::Write(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may has been closed");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    FILE *filp = nullptr;
    filp = streamEntity->fp.get();

    auto [succ, bufGuard, buf, len, hasOffset, offset] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    auto arg = make_shared<AsyncWrtieArg>(move(bufGuard));
    if (!arg) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf = buf, len = len, filp, hasOffset = hasOffset, offset = offset]() -> NError {
        int ret = fseek(filp, static_cast<long>(offset), SEEK_SET);
        if (hasOffset && (ret < 0)) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return NError(errno);
        }
        arg->actLen = fwrite(buf, 1, len, filp);
        if ((arg->actLen == 0) && (arg->actLen != len)) {
            HILOGE("Failed to fwrite stream");
            return NError(EIO);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, static_cast<int64_t>(arg->actLen)) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        hasOp = HasOption(env, funcArg[NARG_POS::SECOND]);
    }
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_WRITE_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_WRITE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value StreamNExporter::Read(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may have been closed");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    FILE *filp = nullptr;
    filp = streamEntity->fp.get();

    auto [succ, buf, len, hasOffset, offset] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncReadArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    if (!arg) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf = buf, len = len, filp, hasOffset = hasOffset, offset = offset]() -> NError {
        int ret = fseek(filp, offset, SEEK_SET);
        if (hasOffset && (ret < 0)) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return NError(errno);
        }
        size_t actLen = fread(buf, 1, len, filp);
        if ((actLen != static_cast<size_t>(len) && !feof(filp)) || ferror(filp)) {
            HILOGE("Invalid buffer size and pointer, actlen: %{public}zu", actLen);
            return NError(EIO);
        } else {
            arg->lenRead = actLen;
            return NError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, arg->lenRead) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        hasOp = HasOption(env, funcArg[NARG_POS::SECOND]);
    }
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_READ_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_READ_NAME, cbExec, cbCompl).val_;
    }
}

napi_value StreamNExporter::Close(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        HILOGE("Stream may has been closed");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    auto fp = NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());
    if (!fp) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_CLOSE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_CLOSE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value StreamNExporter::Constructor(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<StreamEntity> streamEntity = make_unique<StreamEntity>();
    if (!NClass::SetEntityFor<StreamEntity>(env, funcArg.GetThisVar(), move(streamEntity))) {
        HILOGE("INNER BUG. Failed to wrap entity for obj stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool StreamNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("writeSync", WriteSync),
        NVal::DeclareNapiFunction("flush", Flush::Async),
        NVal::DeclareNapiFunction("flushSync", Flush::Sync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("close", Close),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value cls = nullptr;
    tie(succ, cls) = NClass::DefineClass(exports_.env_, className, StreamNExporter::Constructor, move(props));
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, cls);
    if (!succ) {
        HILOGE("INNER BUG. Failed to save class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, cls);
}

string StreamNExporter::GetClassName()
{
    return StreamNExporter::className_;
}

StreamNExporter::StreamNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

StreamNExporter::~StreamNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
