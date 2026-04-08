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
 * WITHOUT WARRANTIES or conditions of any kind, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi/mmap_napi.h"

#include <memory>
#include <tuple>

#include "class_file/file_entity.h"
#include "class_filemapping/fs_filemapping.h"
#include "class_filemapping/napi/filemapping_napi.h"
#include "common_func.h"
#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"
#include "mmap_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

const string PROCEDURE_MMAP_NAME = "fs.mmap";

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)

static tuple<int, int, int, off_t, size_t> ParseMmapArgs(napi_env env, NFuncArg &funcArg)
{
    if (funcArg.GetArgc() < NARG_CNT::FOUR) {
        HILOGE("Number of arguments unmatched");
        return { EINVAL, -1, 0, 0, 0 };
    }

    int fd = -1;
    auto [succFd, fdValue] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (succFd) {
        fd = fdValue;
    } else {
        auto fileEntity = NClass::GetEntityOf<FileEntity>(env, funcArg[NARG_POS::FIRST]);
        if (!fileEntity) {
            HILOGE("Failed to get file entity");
            return { EINVAL, -1, 0, 0, 0 };
        }
        if (!fileEntity->fd_) {
            HILOGE("Invalid fd");
            return { EBADF, -1, 0, 0, 0 };
        }
        fd = fileEntity->fd_->GetFD();
    }

    auto [succMode, mode] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        return { EINVAL, -1, 0, 0, 0 };
    }

    auto [succOffset, offset] = NVal(env, funcArg[NARG_POS::THIRD]).ToInt64();
    if (!succOffset) {
        HILOGE("Failed to convert offset to int64");
        return { EINVAL, -1, 0, 0, 0 };
    }

    auto [succSize, size] = NVal(env, funcArg[NARG_POS::FOURTH]).ToInt32();
    if (!succSize) {
        HILOGE("Failed to convert size to int32");
        return { EINVAL, -1, 0, 0, 0 };
    }

    return { 0, fd, mode, static_cast<off_t>(offset), static_cast<size_t>(size) };
}

static napi_value InstantiateFileMapping(napi_env env, FsFileMapping *mapping)
{
    napi_value objFileMapping = NClass::InstantiateClass(env, FileMappingNapi::className_, {});
    if (!objFileMapping) {
        HILOGE("Failed to instantiate FileMapping class");
        NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).ThrowErr(env);
        return nullptr;
    }

    auto entity = NClass::GetEntityOf<FileMappingEntity>(env, objFileMapping);
    if (!entity) {
        HILOGE("Failed to get FileMappingEntity");
        NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).ThrowErr(env);
        return nullptr;
    }

    auto *coreEntity = mapping->GetEntity();
    entity->fsMapping = mapping;
    entity->mapAddr = coreEntity->mapAddr;
    entity->rawMapAddr = coreEntity->rawMapAddr;
    entity->capacity = coreEntity->capacity;
    entity->rawCapacity = coreEntity->rawCapacity;
    entity->adjustment = coreEntity->adjustment;
    entity->limit = coreEntity->limit;
    entity->position = coreEntity->position;
    entity->mode = coreEntity->mode;
    entity->fd = coreEntity->fd;
    entity->offset = coreEntity->offset;
    entity->isValid = coreEntity->isValid;
    return objFileMapping;
}

struct AsyncMmapContext {
    int fd = -1;
    int mode = 0;
    off_t offset = 0;
    size_t size = 0;
    FsFileMapping *mapping = nullptr;
};

napi_value MmapNapi::Sync(napi_env env, napi_callback_info info)
{
    FileFsTrace traceMmapSync("MmapSync");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [err, fd, mode, offset, size] = ParseMmapArgs(env, funcArg);
    if (err != 0) {
        NError(err).ThrowErr(env);
        return nullptr;
    }

    auto result = MmapCore::DoMmap(fd, mode, offset, size);
    if (!result.IsSuccess()) {
        HILOGE("DoMmap failed");
        NError(result.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }

    FsFileMapping *mapping = result.GetData().value();
    napi_value obj = InstantiateFileMapping(env, mapping);
    if (!obj) {
        delete mapping;
        return nullptr;
    }
    return obj;
}

napi_value MmapNapi::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [err, fd, mode, offset, size] = ParseMmapArgs(env, funcArg);
    if (err != 0) {
        NError(err).ThrowErr(env);
        return nullptr;
    }

    auto ctx = CreateSharedPtr<AsyncMmapContext>();
    if (ctx == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    ctx->fd = fd;
    ctx->mode = mode;
    ctx->offset = offset;
    ctx->size = size;
    ctx->mapping = nullptr;

    auto cbExec = [ctx]() -> NError {
        auto result = MmapCore::DoMmap(ctx->fd, ctx->mode, ctx->offset, ctx->size);
        if (!result.IsSuccess()) {
            return NError(result.GetError().GetErrNo());
        }
        ctx->mapping = result.GetData().value();
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [ctx](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value obj = InstantiateFileMapping(env, ctx->mapping);
        if (!obj) {
            delete ctx->mapping;
            return { env, NError(FILEIO_SYS_CAP_TAG + E_INTERN_RES).GetNapiErr(env) };
        }
        return { env, obj };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MMAP_NAME, cbExec, cbCompl).val_;
}

#else

napi_value MmapNapi::Sync(napi_env env, napi_callback_info info)
{
    HILOGE("mmap is not supported on this platform");
    NError(EOPNOTSUPP).ThrowErr(env);
    return nullptr;
}

napi_value MmapNapi::Async(napi_env env, napi_callback_info info)
{
    HILOGE("mmap is not supported on this platform");
    NError(EOPNOTSUPP).ThrowErr(env);
    return nullptr;
}

#endif

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
