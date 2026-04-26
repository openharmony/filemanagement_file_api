/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "napi/filemapping_napi.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <tuple>

#include <securec.h>

#include "common_func.h"
#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"
#include "fs_filemapping.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

using namespace std;
using namespace OHOS::FileManagement::LibN;

static NError ThrowMmapError(int errCode)
{
    return NError(FILEIO_SYS_CAP_TAG + errCode);
}

static FsFileMapping *GetMapping(napi_env env, napi_value obj)
{
    auto entity = NClass::GetEntityOf<FileMappingEntity>(env, obj);
    if (!entity || !entity->fsMapping) {
        HILOGE("Failed to get file mapping");
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    return entity->fsMapping;
}

static constexpr size_t BUFFER_ARG_OFFSET = 1;
static constexpr size_t LENGTH_ARG_OFFSET = 2;

static bool ParsePositionArg(napi_env env, NFuncArg &funcArg, FileMappingEntity *entity,
    ReadWriteArgs &args)
{
    if (!NVal(env, funcArg[NARG_POS::FIRST]).TypeIs(napi_number)) {
        return true;
    }
    auto [succ, pos] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || pos < 0 || static_cast<size_t>(pos) > entity->capacity) {
        HILOGE("Invalid position value");
        NError(EINVAL).ThrowErr(env);
        return false;
    }
    args.position = static_cast<size_t>(pos);
    args.bufferArgIndex = BUFFER_ARG_OFFSET;
    args.lengthArgIndex = LENGTH_ARG_OFFSET;
    return true;
}

static bool GetBufferArg(napi_env env, NFuncArg &funcArg, ReadWriteArgs &args,
    size_t argc, const char *errorMsg)
{
    if (args.bufferArgIndex >= argc) {
        HILOGE("%{public}s", errorMsg);
        NError(EINVAL).ThrowErr(env);
        return false;
    }
    auto [succ, buffer, bufLen] = NVal(env, funcArg[args.bufferArgIndex]).ToArraybuffer();
    if (!succ || buffer == nullptr) {
        HILOGE("Invalid buffer argument");
        NError(EINVAL).ThrowErr(env);
        return false;
    }
    args.buffer = buffer;
    args.bufLen = bufLen;
    args.opLength = bufLen;
    return true;
}

static void ParseLengthArg(napi_env env, NFuncArg &funcArg, ReadWriteArgs &args, size_t argc)
{
    if (args.lengthArgIndex >= argc) {
        return;
    }
    auto [succ, len] = NVal(env, funcArg[args.lengthArgIndex]).ToInt32();
    if (succ && len >= 0) {
        args.opLength = static_cast<size_t>(len);
    }
}

static bool ValidateLength(napi_env env, FileMappingEntity *entity, ReadWriteArgs &args)
{
    size_t remaining = (args.position < entity->limit) ? (entity->limit - args.position) : 0;
    if (args.opLength > remaining) {
        ThrowMmapError(E_MMAP_OOB).ThrowErr(env);
        return false;
    }
    if (args.opLength > args.bufLen) {
        args.opLength = args.bufLen;
    }
    return true;
}

napi_value FileMappingNapi::SetPosition(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto [succ, newPosition] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || newPosition < 0 || static_cast<size_t>(newPosition) > mapping->GetEntity()->limit) {
        HILOGE("Invalid position value");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->SetPosition(static_cast<size_t>(newPosition));
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileMappingNapi::GetPosition(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->GetPosition();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(ret.GetData().value())).val_;
}

napi_value FileMappingNapi::Capacity(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->Capacity();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(ret.GetData().value())).val_;
}

napi_value FileMappingNapi::SetLimit(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto [succ, newLimit] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || newLimit < 0 || static_cast<size_t>(newLimit) > mapping->GetEntity()->capacity) {
        HILOGE("Invalid limit value");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->SetLimit(static_cast<size_t>(newLimit));
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileMappingNapi::GetLimit(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->GetLimit();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(ret.GetData().value())).val_;
}

napi_value FileMappingNapi::Flip(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->Flip();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileMappingNapi::Remaining(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto ret = mapping->Remaining();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(ret.GetData().value())).val_;
}

napi_value FileMappingNapi::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    auto *entity = mapping->GetEntity();
    ReadWriteArgs args = { entity->position, 0, 1, nullptr, 0, 0 };
    if (!ParsePositionArg(env, funcArg, entity, args)) {
        return nullptr;
    }
    if (!GetBufferArg(env, funcArg, args, funcArg.GetArgc(), "Missing buffer argument")) {
        return nullptr;
    }
    ParseLengthArg(env, funcArg, args, funcArg.GetArgc());
    if (!ValidateLength(env, entity, args)) {
        return nullptr;
    }
    if (args.bufferArgIndex == 0) {
        auto ret = mapping->Read(args.buffer, args.bufLen, args.opLength);
        if (!ret.IsSuccess()) {
            HILOGE("Read failed");
            NError(ret.GetError().GetErrNo()).ThrowErr(env);
            return nullptr;
        }
    } else {
        auto ret = mapping->ReadFrom(args.position, args.buffer, args.bufLen, args.opLength);
        if (!ret.IsSuccess()) {
            HILOGE("ReadFrom failed");
            NError(ret.GetError().GetErrNo()).ThrowErr(env);
            return nullptr;
        }
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(args.opLength)).val_;
}

napi_value FileMappingNapi::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    if (mapping->IsReadOnly()) {
        ThrowMmapError(E_MMAP_RO).ThrowErr(env);
        return nullptr;
    }
    auto *entity = mapping->GetEntity();
    ReadWriteArgs args = { entity->position, 0, 1, nullptr, 0, 0 };
    if (!ParsePositionArg(env, funcArg, entity, args)) {
        return nullptr;
    }
    if (!GetBufferArg(env, funcArg, args, funcArg.GetArgc(), "Missing data argument")) {
        return nullptr;
    }
    ParseLengthArg(env, funcArg, args, funcArg.GetArgc());
    if (!ValidateLength(env, entity, args)) {
        return nullptr;
    }
    if (args.bufferArgIndex == 0) {
        auto ret = mapping->Write(args.buffer, args.bufLen, args.opLength);
        if (!ret.IsSuccess()) {
            HILOGE("Write failed");
            NError(ret.GetError().GetErrNo()).ThrowErr(env);
            return nullptr;
        }
    } else {
        auto ret = mapping->WriteTo(args.position, args.buffer, args.bufLen, args.opLength);
        if (!ret.IsSuccess()) {
            HILOGE("WriteTo failed");
            NError(ret.GetError().GetErrNo()).ThrowErr(env);
            return nullptr;
        }
    }
    return NVal::CreateInt32(env, static_cast<int32_t>(args.opLength)).val_;
}

napi_value FileMappingNapi::Msync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    off_t msyncOffset = 0;
    size_t msyncLength = mapping->GetEntity()->capacity;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        auto [succOffset, offset] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
        auto [succLen, len] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succOffset || !succLen || offset < 0 || len < 0) {
            HILOGE("Invalid msync arguments");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        msyncOffset = static_cast<off_t>(offset);
        msyncLength = static_cast<size_t>(len);
    }
    auto cbExec = [mapping, msyncOffset, msyncLength]() -> NError {
        auto ret = mapping->Msync(static_cast<size_t>(msyncOffset), msyncLength);
        if (!ret.IsSuccess()) {
            return NError(ret.GetError().GetErrNo());
        }
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MSYNC_NAME, cbExec, cbCompl).val_;
}

napi_value FileMappingNapi::MsyncSync(napi_env env, napi_callback_info info)
{
    FileFsTrace traceMsyncSync("MsyncSync");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping || !mapping->CheckValid()) {
        if (!mapping) {
            return nullptr;
        }
        ThrowMmapError(E_MMAP_FREE).ThrowErr(env);
        return nullptr;
    }
    off_t msyncOffset = 0;
    size_t msyncLength = mapping->GetEntity()->capacity;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        auto [succOffset, offset] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
        auto [succLen, len] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succOffset || !succLen || offset < 0 || len < 0) {
            HILOGE("Invalid msync arguments");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        msyncOffset = static_cast<off_t>(offset);
        msyncLength = static_cast<size_t>(len);
    }
    auto ret = mapping->Msync(static_cast<size_t>(msyncOffset), msyncLength);
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileMappingNapi::Unmap(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping) {
        return nullptr;
    }
    auto cbExec = [mapping]() -> NError {
        auto ret = mapping->Unmap();
        if (!ret.IsSuccess()) {
            return NError(ret.GetError().GetErrNo());
        }
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_UNMAP_NAME, cbExec, cbCompl).val_;
}

napi_value FileMappingNapi::UnmapSync(napi_env env, napi_callback_info info)
{
    FileFsTrace traceUnmapSync("UnmapSync");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto mapping = GetMapping(env, funcArg.GetThisVar());
    if (!mapping) {
        return nullptr;
    }
    auto ret = mapping->Unmap();
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileMappingNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto entity = CreateUniquePtr<FileMappingEntity>();
    if (entity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    if (!NClass::SetEntityFor<FileMappingEntity>(env, funcArg.GetThisVar(), move(entity))) {
        HILOGE("Failed to set file mapping entity");
        NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool FileMappingNapi::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("setPosition", SetPosition),
        NVal::DeclareNapiFunction("getPosition", GetPosition),
        NVal::DeclareNapiFunction("capacity", Capacity),
        NVal::DeclareNapiFunction("setLimit", SetLimit),
        NVal::DeclareNapiFunction("getLimit", GetLimit),
        NVal::DeclareNapiFunction("flip", Flip),
        NVal::DeclareNapiFunction("remaining", Remaining),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("msync", Msync),
        NVal::DeclareNapiFunction("msyncSync", MsyncSync),
        NVal::DeclareNapiFunction("unmap", Unmap),
        NVal::DeclareNapiFunction("unmapSync", UnmapSync),
    };
    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        FileMappingNapi::Constructor, move(props));
    if (!succ) {
        HILOGE("Define class exceptions");
        NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).ThrowErr(exports_.env_);
        return false;

    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Save class exceptions");
        NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).ThrowErr(exports_.env_);
        return false;
    }
    return exports_.AddProp(className, classValue);
}

string FileMappingNapi::GetClassName()
{
    return FileMappingNapi::className_;
}

FileMappingNapi::FileMappingNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}

FileMappingNapi::~FileMappingNapi() {}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
