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

#include "randomaccessfile_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <uv.h>

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "randomaccessfile_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static RandomAccessFileEntity *GetRAFEntity(napi_env env, napi_value raf_entity)
{
    auto rafEntity = NClass::GetEntityOf<RandomAccessFileEntity>(env, raf_entity);
    if (!rafEntity) {
        UniError(EINVAL).ThrowErr(env, "Cannot get entity of RandomAccessFile");
        return nullptr;
    }
    if (rafEntity->fd_ == 0) {
        UniError(EINVAL).ThrowErr(env, "RandomAccessFile has been closed yet");
        return nullptr;
    }
    return rafEntity;
}

static tuple<bool, int> ParseJsFP(napi_env env, napi_value FPFromJs)
{
    auto [succ, fp] = NVal(env, FPFromJs).ToInt32();
    return { succ, fp };
}

static tuple<bool, void*, size_t, bool, size_t, size_t> GetRAFReadArg(napi_env env,
    napi_value ReadBuf, napi_value option)
{
    bool succ = false;
    bool hasPos = false;
    void *buf = nullptr;
    size_t len = 0;
    size_t pos = 0;
    size_t offset = 0;
    tie(succ, buf, len, hasPos, pos, offset) = CommonFunc::GetReadArg(env, ReadBuf, option);
    if (!succ) {
        return { false, nullptr, 0, false, 0, 0 };
    }
    return { true, buf, len, hasPos, pos, offset };
}

static tuple<bool, void *, size_t, bool, size_t> GetRAFWriteArg(napi_env env,
    napi_value WriteBuf, napi_value option)
{
    bool succ = false;
    bool hasPos = false;
    void *buf = nullptr;
    size_t len = 0;
    size_t pos = 0;
    tie(succ, ignore, buf, len, hasPos, pos) = CommonFunc::GetWriteArg(env, WriteBuf, option);
    if (!succ) {
        return  { false, nullptr, 0, false, 0 };
    }
    return { true, buf, len, hasPos, pos };
}

static int DoReadRAF(napi_env env, void* buf, size_t len, int fd, size_t pos)
{
    uv_loop_s *loop = nullptr;
    uv_fs_t read_req;
    napi_get_uv_event_loop(env, &loop);
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    int ret = uv_fs_read(loop, &read_req, fd, &iov, 1, pos, NULL);
    uv_fs_req_cleanup(&read_req);
    return ret;
}

static size_t DoWriteRAF(napi_env env, void* buf, size_t len, int fd, size_t pos)
{
    uv_loop_s *loop = nullptr;
    uv_fs_t write_req;
    napi_get_uv_event_loop(env, &loop);
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    size_t ret = uv_fs_write(loop, &write_req, fd, &iov, 1, pos, NULL);
    uv_fs_req_cleanup(&write_req);
    return ret;
}

napi_value RandomAccessFileNExporter::GetFD(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    return NVal::CreateInt32(env, rafEntity->fd_.get()->GetFD()).val_;
}

napi_value RandomAccessFileNExporter::GetFPointer(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    return NVal::CreateInt64(env, rafEntity->fpointer).val_;
}

napi_value RandomAccessFileNExporter::SetFilePointerSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    auto [succ, fp] = ParseJsFP(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fpointer");
        return nullptr;
    }
    rafEntity->fpointer = fp;
    return NVal::CreateUndefined(env).val_;
}

napi_value RandomAccessFileNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    auto [succ, buf, len, hasPos, pos, ignore] =
        GetRAFReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid buffer/options");
        return nullptr;
    }
    int actLen = DoReadRAF(env, buf, len, rafEntity->fd_.get()->GetFD(), rafEntity->fpointer + pos);
    if (actLen < 0) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    rafEntity->fpointer += actLen;
    return NVal::CreateInt64(env, actLen).val_;
}

struct AsyncIOReadArg {
    size_t lenRead { 0 };
    int offset { 0 };
    NRef refReadBuf;

    explicit AsyncIOReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncIOReadArg() = default;
};

static napi_value ReadExec(napi_env env, NFuncArg &funcArg)
{
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    bool succ = false;
    bool hasPos = false;
    size_t len = 0;
    size_t pos = 0;
    size_t offset = 0;
    void* buf = nullptr;
    tie(succ, buf, len, hasPos, pos, offset) = GetRAFReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid buffer/options");
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    auto cbExec = [arg, buf, len, hasPos, pos, offset, rafEntity](napi_env env) -> UniError {
        int actLen = DoReadRAF(env, buf, len, rafEntity->fd_.get()->GetFD(), rafEntity->fpointer + pos);
        if (actLen < 0) {
            return UniError(errno);
        }
        arg->lenRead = actLen;
        arg->offset = offset;
        rafEntity->fpointer += actLen;
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal obj = NVal::CreateObject(env);
        obj.AddProp(
            {   NVal::DeclareNapiProperty("bytesRead", NVal::CreateInt64(env, arg->lenRead).val_),
                NVal::DeclareNapiProperty("buffer", arg->refReadBuf.Deref(env).val_),
                NVal::DeclareNapiProperty("offset", NVal::CreateInt64(env, arg->offset).val_)
            });
        return { obj };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(readProcedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(readProcedureName, cbExec, cbCompl).val_;
    }
}

napi_value RandomAccessFileNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    return ReadExec(env, funcArg);
}

napi_value RandomAccessFileNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }

    bool succ = false;
    bool hasPos = false;
    void *buf = nullptr;
    size_t len = 0;
    size_t pos = 0;
    tie(succ, buf, len, hasPos, pos) = GetRAFWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid buffer/options");
        return nullptr;
    }
    if (hasPos) {
        pos = rafEntity->fpointer + pos;
    } else {
        pos = rafEntity->fpointer;
    }
    ssize_t writeLen = DoWriteRAF(env, buf, len, rafEntity->fd_.get()->GetFD(), pos);
    if (writeLen < 0) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    rafEntity->fpointer += writeLen;
    return NVal::CreateInt64(env, writeLen).val_;
}

struct AsyncIOWriteArg {
    NRef refWriteArrayBuf_;
    size_t actLen = 0;
    explicit AsyncIOWriteArg(NVal refWriteArrayBuf) : refWriteArrayBuf_(refWriteArrayBuf) {}
    ~AsyncIOWriteArg() = default;
};

napi_value RandomAccessFileNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    bool succ = false;
    bool hasPos = false;
    void *buf = nullptr;
    size_t len = 0;
    size_t pos = 0;
    tie(succ, buf, len, hasPos, pos) = GetRAFWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid buffer/options");
        return nullptr;
    }
    if (hasPos) {
        pos = rafEntity->fpointer + pos;
    } else {
        pos = rafEntity->fpointer;
    }

    auto arg = make_shared<AsyncIOWriteArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    auto cbExec = [arg, buf, len, fd = rafEntity->fd_.get()->GetFD(), pos, rafEntity](napi_env env) -> UniError {
        size_t writeLen = DoWriteRAF(env, buf, len, fd, pos);
        if (writeLen < 0) {
            return UniError(errno);
        }
        arg->actLen = writeLen;
        rafEntity->fpointer += writeLen;
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, arg->actLen) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(writeProcedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(writeProcedureName, cbExec, cbCompl).val_;
    }
}

napi_value RandomAccessFileNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto rafEntity = GetRAFEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    rafEntity->fd_.reset();
    return NVal::CreateUndefined(env).val_;
}

napi_value RandomAccessFileNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    if (!NClass::SetEntityFor<RandomAccessFileEntity>(env, funcArg.GetThisVar(), move(rafEntity))) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj RandomAccessFile");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool RandomAccessFileNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
        NVal::DeclareNapiFunction("setFilePointerSync", SetFilePointerSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
        NVal::DeclareNapiGetter("fd", GetFD),
        NVal::DeclareNapiGetter("fpointer", GetFPointer),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        RandomAccessFileNExporter::Constructor, move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string RandomAccessFileNExporter::GetClassName()
{
    return RandomAccessFileNExporter::className_;
}

RandomAccessFileNExporter::RandomAccessFileNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

RandomAccessFileNExporter::~RandomAccessFileNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS