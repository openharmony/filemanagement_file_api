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

#include "prop_n_exporter_v9.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>
#include <ctime>

#include "../common_func.h"
#include "open_v9.h"
#include "stat_v9.h"
#include "truncate_v9.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value PropNExporterV9::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    int64_t len = 0;
    bool hasPos = false;
    int64_t pos = 0;
    tie(succ, buf, len, hasPos, pos) =
        CommonFunc::GetReadArgV9(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t actLen;
    if (hasPos) {
        actLen = pread(fd, buf, len, pos);
    } else {
        actLen = read(fd, buf, len);
    }
    if (actLen == -1) {
        UniError(errno, true).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, actLen).val_;
}

struct AsyncIOReadArg {
    ssize_t lenRead { 0 };
};

static UniError ReadExec(shared_ptr<AsyncIOReadArg> arg, void *buf, size_t len, int fd, size_t position)
{
    if (position == static_cast<size_t>(INVALID_POSITION)) {
        arg->lenRead = read(fd, buf, len);
    } else {
        arg->lenRead = pread(fd, buf, len, position);
    }

    if (arg->lenRead == -1) {
        return UniError(errno, true);
    } else {
        return UniError(ERRNO_NOERR);
    }
}

napi_value PropNExporterV9::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    int64_t len = 0;
    int fd = 0;
    bool hasPos = false;
    int64_t pos = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    tie(succ, buf, len, hasPos, pos) =
        CommonFunc::GetReadArgV9(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>();
    auto cbExec = [arg, buf, len, fd, pos](napi_env env) -> UniError {
        return ReadExec(arg, buf, len, fd, pos);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, arg->lenRead) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") ||  op.HasProp("length")|| !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIORead", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIORead", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

UniError PropNExporterV9::WriteExec(shared_ptr<AsyncIOWrtieArg> arg, void *buf, size_t len, int fd, size_t position)
{
    if (position == static_cast<size_t>(INVALID_POSITION)) {
        arg->actLen = write(fd, buf, len);
    } else {
        arg->actLen = pwrite(fd, buf, len, position);
    }

    if (arg->actLen == -1) {
        return UniError(errno, true);
    } else {
        return UniError(ERRNO_NOERR);
    }
}

napi_value PropNExporterV9::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<char[]> bufGuard;
    void *buf = nullptr;
    size_t len = 0;
    size_t position = 0;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArgV9(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    shared_ptr<AsyncIOWrtieArg> arg;
    if (bufGuard) {
        arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    } else {
        arg = make_shared<AsyncIOWrtieArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    }
    auto cbExec = [arg, buf, len, fd, position](napi_env env) -> UniError {
        return WriteExec(arg, buf, len, fd, position);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, arg->actLen) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length") ||
            op.HasProp("encoding") || !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }

    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporterV9::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    size_t position = 0;
    unique_ptr<char[]> bufGuard;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArgV9(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t writeLen;
    if (position == static_cast<size_t>(INVALID_POSITION)) {
        writeLen = write(fd, buf, len);
    } else {
        writeLen = pwrite(fd, buf, len, position);
    }

    if (writeLen == -1) {
        UniError(errno, true).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

bool PropNExporterV9::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("open", OpenV9::Async),
        NVal::DeclareNapiFunction("openSync", OpenV9::Sync),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("stat", StatV9::Async),
        NVal::DeclareNapiFunction("statSync", StatV9::Sync),
        NVal::DeclareNapiFunction("truncate", TruncateV9::Async),
        NVal::DeclareNapiFunction("truncateSync", TruncateV9::Sync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
    });
}

string PropNExporterV9::GetClassName()
{
    return PropNExporterV9::className_;
}

PropNExporterV9::PropNExporterV9(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporterV9::~PropNExporterV9() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
