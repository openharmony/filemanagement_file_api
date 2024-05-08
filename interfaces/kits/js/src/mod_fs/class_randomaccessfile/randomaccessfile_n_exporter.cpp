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

#include "randomaccessfile_n_exporter.h"

#include "file_utils.h"
#include "randomaccessfile_entity.h"
#include "../common_func.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, RandomAccessFileEntity*> GetRAFEntity(napi_env env, napi_value raf_entity)
{
    auto rafEntity = NClass::GetEntityOf<RandomAccessFileEntity>(env, raf_entity);
    if (!rafEntity) {
        return { false, nullptr };
    }
    return { true, rafEntity };
}

static int DoReadRAF(napi_env env, void* buf, size_t len, int fd, int64_t offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (read_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}

static int DoWriteRAF(napi_env env, void* buf, size_t len, int fd, int64_t offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> write_req = {
        new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (write_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}

napi_value RandomAccessFileNExporter::GetFD(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt32(env, rafEntity->fd.get()->GetFD()).val_;
}

napi_value RandomAccessFileNExporter::GetFPointer(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, rafEntity->filePointer).val_;
}

napi_value RandomAccessFileNExporter::SetFilePointerSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fp] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt64();
    if (!succ) {
        HILOGE("Invalid filePointer");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    rafEntity->filePointer = fp;
    return NVal::CreateUndefined(env).val_;
}

static int64_t CalculateOffset(int64_t offset, int64_t fPointer)
{
    if (offset < 0) {
        HILOGD("No specified offset provided");
        offset = fPointer;
    } else {
        offset += fPointer;
    }
    return offset;
}

napi_value RandomAccessFileNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    auto [succ, buf, len, offset] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    int actLen = DoReadRAF(env, buf, len, rafEntity->fd.get()->GetFD(), offset);
    if (actLen < 0) {
        HILOGE("Failed to read file for %{public}d", actLen);
        NError(actLen).ThrowErr(env);
        return nullptr;
    }
    rafEntity->filePointer = offset + actLen;
    return NVal::CreateInt64(env, actLen).val_;
}

struct AsyncIORafReadArg {
    int lenRead { 0 };
    NRef rafRefReadBuf;

    explicit AsyncIORafReadArg(NVal jsReadBuf) : rafRefReadBuf(jsReadBuf) {}
    ~AsyncIORafReadArg() = default;
};

static napi_value ReadExec(napi_env env, NFuncArg &funcArg, RandomAccessFileEntity* rafEntity)
{
    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = 0;
    tie(succ, buf, len, offset) = CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncIORafReadArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    auto cbExec = [env, arg, buf, len, offset, rafEntity]() -> NError {
        if (!rafEntity || !rafEntity->fd.get()) {
            HILOGE("RandomAccessFile has been closed in read cbExec possibly");
            return NError(EIO);
        }
        int actLen = DoReadRAF(env, buf, len, rafEntity->fd.get()->GetFD(), offset);
        if (actLen < 0) {
            return NError(actLen);
        }
        arg->lenRead = actLen;
        rafEntity->filePointer = offset + actLen;
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, static_cast<int64_t>(arg->lenRead)) };
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
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto[succ, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succ) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return ReadExec(env, funcArg, rafEntity);
}

napi_value RandomAccessFileNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    auto [succ, ignore, buf, len, offset] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    int writeLen = DoWriteRAF(env, buf, len, rafEntity->fd.get()->GetFD(), offset);
    if (writeLen < 0) {
        NError(writeLen).ThrowErr(env);
        return nullptr;
    }
    rafEntity->filePointer = offset + writeLen;
    return NVal::CreateInt64(env, writeLen).val_;
}

struct AsyncIORafWriteArg {
    NRef rafRefWriteArrayBuf;
    int actLen = 0;
    explicit AsyncIORafWriteArg(NVal refWriteArrayBuf) : rafRefWriteArrayBuf(refWriteArrayBuf) {}
    ~AsyncIORafWriteArg() = default;
};

static napi_value WriteExec(napi_env env, NFuncArg &funcArg, RandomAccessFileEntity* rafEntity)
{
    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = 0;
    tie(succ, ignore, buf, len, offset) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncIORafWriteArg>(NVal(env, funcArg[NARG_POS::FIRST]));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    auto cbExec = [env, arg, buf, len, fd = rafEntity->fd.get()->GetFD(), offset, rafEntity]() -> NError {
        if (!rafEntity || !rafEntity->fd.get()) {
            HILOGE("RandomAccessFile has been closed in write cbExec possibly");
            return NError(EIO);
        }
        int writeLen = DoWriteRAF(env, buf, len, fd, offset);
        if (writeLen < 0) {
            HILOGE("Failed to write file for %{public}d", writeLen);
            return NError(writeLen);
        }
        arg->actLen = writeLen;
        rafEntity->filePointer = offset + writeLen;
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
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

napi_value RandomAccessFileNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto[succ, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succ) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return WriteExec(env, funcArg, rafEntity);
}

static NError CloseFd(int fd)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> close_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!close_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_close(nullptr, close_req.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to close file with ret: %{public}d", ret);
        return NError(ret);
    }
    return NError(ERRNO_NOERR);
}

napi_value RandomAccessFileNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, rafEntity] = GetRAFEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    auto err = CloseFd(rafEntity->fd.get()->GetFD());
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }
    auto fp = NClass::RemoveEntityOfFinal<RandomAccessFileEntity>(env, funcArg.GetThisVar());
    if (!fp) {
        HILOGE("Failed to remove entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value RandomAccessFileNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto rafEntity = CreateUniquePtr<RandomAccessFileEntity>();
    if (rafEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    if (!NClass::SetEntityFor<RandomAccessFileEntity>(env, funcArg.GetThisVar(), move(rafEntity))) {
        HILOGE("INNER BUG. Failed to wrap entity for obj RandomAccessFile");
        NError(EIO).ThrowErr(env);
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
        NVal::DeclareNapiFunction("setFilePointer", SetFilePointerSync),
        NVal::DeclareNapiFunction("close", CloseSync),
        NVal::DeclareNapiGetter("fd", GetFD),
        NVal::DeclareNapiGetter("filePointer", GetFPointer),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        RandomAccessFileNExporter::Constructor, move(props));
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
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
} // namespace FileManagement
} // namespace OHOS