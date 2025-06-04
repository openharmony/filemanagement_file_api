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

#include "stream_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include "flush.h"
#include "securec.h"

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "stream_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value StreamNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    /* To get entity */
    bool succ = false;
    FILE *filp = nullptr;
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may have been closed");
        return nullptr;
    } else {
        filp = streamEntity->fp.get();
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t pos = -1;
    tie(succ, buf, len, pos, ignore) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }

    if (pos >= 0 && (fseek(filp, static_cast<long>(pos), SEEK_SET) == -1)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    size_t actLen = fread(buf, 1, len, filp);
    if ((actLen != len && !feof(filp)) || ferror(filp)) {
        UniError(errno).ThrowErr(env);
    }

    return NVal::CreateInt64(env, actLen).val_;
}

napi_value StreamNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EINVAL).ThrowErr(env, "Stream may have been closed yet");
        return nullptr;
    }
    streamEntity->fp.reset();
    (void)NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());

    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    FILE *filp = nullptr;
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may has been closed");
        return nullptr;
    } else {
        filp = streamEntity->fp.get();
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t position = -1;
    unique_ptr<char[]> bufGuard = nullptr;
    tie(succ, bufGuard, buf, len, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }
    if (position >= 0 && (fseek(filp, static_cast<long>(position), SEEK_SET) == -1)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    size_t writeLen = fwrite(buf, 1, len, filp);
    if ((writeLen == 0) && (writeLen != len)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

struct AsyncWriteArg {
    NRef refWriteArrayBuf;
    unique_ptr<char[]> guardWriteStr = nullptr;
    size_t actLen { 0 };

    explicit AsyncWriteArg(NVal refWriteArrayBuf) : refWriteArrayBuf(refWriteArrayBuf) {}
    explicit AsyncWriteArg(unique_ptr<char[]> &&guardWriteStr) : guardWriteStr(move(guardWriteStr)) {}
    ~AsyncWriteArg() = default;
};

napi_value StreamNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    FILE *filp = nullptr;
    int64_t position = -1;
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may has been closed");
        return nullptr;
    }
    filp = streamEntity->fp.get();

    unique_ptr<char[]> bufGuard = nullptr;
    void *buf = nullptr;
    size_t len = 0;
    tie(succ, bufGuard, buf, len, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<AsyncWriteArg>(move(bufGuard));
    auto cbExec = [arg, buf, len, filp, position](napi_env env) -> UniError {
        if (position >= 0 && (fseek(filp, static_cast<long>(position), SEEK_SET) == -1)) {
            UniError(errno).ThrowErr(env);
            return UniError(errno);
        }
        arg->actLen = fwrite(buf, 1, len, filp);
        if ((arg->actLen == 0) && (arg->actLen != len)) {
            return UniError(errno);
        }
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, arg->actLen) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOStreamWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOStreamWrite", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

struct AsyncReadArg {
    size_t lenRead { 0 };
    NRef refReadBuf;
    int64_t offset { 0 };

    explicit AsyncReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncReadArg() = default;
};

napi_value StreamNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    /* To get entity */
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may have been closed");
        return nullptr;
    }
    FILE *filp = nullptr;
    filp = streamEntity->fp.get();

    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int64_t position = -1;
    int64_t offset = 0;
    tie(succ, buf, len, position, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<AsyncReadArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    auto cbExec = [arg, buf, position, filp, len, offset](napi_env env) -> UniError {
        if (position >= 0 && (fseek(filp, static_cast<long>(position), SEEK_SET) == -1)) {
            UniError(errno).ThrowErr(env);
            return UniError(errno);
        }
        size_t actLen = fread(buf, 1, len, filp);
        if ((actLen != len && !feof(filp)) || ferror(filp)) {
            return UniError(errno);
        } else {
            arg->lenRead = actLen;
            arg->offset = offset;
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal obj = NVal::CreateObject(env);
        obj.AddProp({
            NVal::DeclareNapiProperty("bytesRead", NVal::CreateInt64(env, arg->lenRead).val_),
            NVal::DeclareNapiProperty("buffer", arg->refReadBuf.Deref(env).val_),
            NVal::DeclareNapiProperty("offset", NVal::CreateInt64(env, arg->offset).val_)
            });
        return { obj };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOStreamRead", cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOStreamRead", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::Close(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may has been closed");
        return nullptr;
    }

    auto fp = NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());
    if (!fp) {
        UniError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [](napi_env env) -> UniError {
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    string procedureName = "FileIOStreamClose";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    unique_ptr<StreamEntity> streamEntity = make_unique<StreamEntity>();
    if (!NClass::SetEntityFor<StreamEntity>(env, funcArg.GetThisVar(), move(streamEntity))) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj stream");
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
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, cls);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
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
} // namespace DistributedFS
} // namespace OHOS
