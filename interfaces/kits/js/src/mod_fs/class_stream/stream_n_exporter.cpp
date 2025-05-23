/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "stream_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
std::mutex StreamNExporter::mutex;
std::shared_ptr<FILE> StreamNExporter::GetFilePtr(StreamEntity *streamEntity)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (streamEntity) {
        return streamEntity->fp;
    }
    return nullptr;
}

StreamEntity* StreamNExporter::GetEntityOf(napi_env env, NFuncArg &funcArg)
{
    NClass &nClass = NClass::GetInstance();
    lock_guard<std::mutex>(nClass.wrapLock);
    if (nClass.wrapReleased) {
        return nullptr;
    }
    return NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
}

napi_value StreamNExporter::FlushSync(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    int ret = fflush(fp.get());
    if (ret < 0) {
        HILOGE("Failed to fflush file in the stream, ret: %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::Flush(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [fp]() -> NError {
        if (!fp) {
            HILOGE("Stream has been closed in flush cbExec possibly");
            return NError(EIO);
        }
        int ret = fflush(fp.get());
        if (ret < 0) {
            HILOGE("Failed to fflush file in the stream");
            return NError(errno);
        } else {
            return NError(ERRNO_NOERR);
        }
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_FLUSH_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_FLUSH_NAME, cbExec, cbCompl).val_;
    }
}

napi_value StreamNExporter::ReadSync(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    auto [succ, buf, len, offset] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    if (offset >= 0) {
        int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    }

    size_t actLen = fread(buf, 1, len, fp.get());
    if ((actLen != static_cast<size_t>(len) && !feof(fp.get())) || ferror(fp.get())) {
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
    if (!streamEntity) {
        HILOGE("Failed to get entity of Stream, may closed twice");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(mutex);
        (void)NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());
    }
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

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    auto [succ, bufGuard, buf, len, offset] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }
    if (offset >= 0) {
        int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    }

    size_t writeLen = fwrite(buf, 1, len, fp.get());
    if ((writeLen == 0) && (writeLen != len)) {
        HILOGE("Failed to fwrite stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(writeLen)).val_;
}

static napi_value WriteExec(napi_env env, NFuncArg &funcArg, shared_ptr<FILE> fp)
{
    auto [succ, bufGuard, buf, len, offset] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncWriteArg>(move(bufGuard));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf = buf, len = len, fp, offset = offset]() -> NError {
        if (!fp.get()) {
            HILOGE("Stream has been closed in write cbExec possibly");
            return NError(EIO);
        }
        if (offset >= 0) {
            int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
            if (ret < 0) {
                HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
                return NError(errno);
            }
        }
        arg->actLen = fwrite(buf, 1, len, fp.get());
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
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_WRITE_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_WRITE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value StreamNExporter::Write(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return WriteExec(env, funcArg, fp);
}

static napi_value ReadExec(napi_env env, NFuncArg &funcArg, shared_ptr<FILE> fp)
{
    auto [succ, buf, len, offset] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncReadArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf = buf, len = len, fp, offset = offset]() -> NError {
        if (!fp.get()) {
            HILOGE("Stream has been closed in read cbExec possibly");
            return NError(EIO);
        }
        if (offset >= 0) {
            if (fseek(fp.get(), static_cast<long>(offset), SEEK_SET) < 0) {
                HILOGE("Failed to set the offset location of the file stream pointer");
                return NError(errno);
            }
        }
        size_t actLen = fread(buf, 1, len, fp.get());
        if ((actLen != static_cast<size_t>(len) && !feof(fp.get())) || ferror(fp.get())) {
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
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STREAM_READ_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STREAM_READ_NAME, cbExec, cbCompl).val_;
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

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return ReadExec(env, funcArg, fp);
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
    if (!streamEntity) {
        HILOGE("Failed to get entity of Stream, may closed twice");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    StreamEntity* ret = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex);
        ret = NClass::RemoveEntityOfFinal<StreamEntity>(env, funcArg.GetThisVar());
    }
    if (!ret) {
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

napi_value StreamNExporter::Seek(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = GetEntityOf(env, funcArg);
    if (streamEntity == nullptr) {
        NError(UNKROWN_ERR).ThrowErr(env);
        return nullptr;
    }
    auto fp = GetFilePtr(streamEntity);
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    auto [succGetOffset, offset] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt64();
    if (!succGetOffset) {
        HILOGE("Invalid offset from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int whence = SEEK_SET;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        auto [succGetWhence, pos] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(SEEK_SET);
        if (!succGetWhence || pos < SEEK_SET || pos > SEEK_END) {
            HILOGE("Invalid whence from JS third argument");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        whence = pos;
    }

    if (offset >= 0) {
        int ret = fseek(fp.get(), static_cast<long>(offset), whence);
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    }
    int64_t res = ftell(fp.get());
    if (res < 0) {
        HILOGE("Failed to tell, error:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, res).val_;
}

napi_value StreamNExporter::Constructor(napi_env env, napi_callback_info cbInfo)
{
    NFuncArg funcArg(env, cbInfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto streamEntity = CreateUniquePtr<StreamEntity>();
    if (streamEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
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
        NVal::DeclareNapiFunction("flush", Flush),
        NVal::DeclareNapiFunction("flushSync", FlushSync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("close", Close),
        NVal::DeclareNapiFunction("seek", Seek),
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
