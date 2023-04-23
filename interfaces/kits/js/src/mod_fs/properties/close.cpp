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

#include "close.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static FileEntity *GetFileEntity(napi_env env, napi_value objFile)
{
    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return nullptr;
    }
    if (!fileEntity->fd_) {
        HILOGE("The fd of rafEntity is not exist");
        return nullptr;
    }
    return fileEntity;
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
        return NError(errno);
    }
    return NError(ERRNO_NOERR);
}

static tuple<bool, FileStruct> ParseJsOperand(napi_env env, napi_value fdOrFileFromJsArg)
{
    auto [isFd, fd] = NVal(env, fdOrFileFromJsArg).ToInt32();
    if (isFd && fd >= 0) {
        return { true, FileStruct { true, fd, nullptr } };
    }
    if (isFd && fd < 0) {
        return { false, FileStruct { false, -1, nullptr } };
    }
    auto file = GetFileEntity(env, fdOrFileFromJsArg);
    if (file) {
        return { true, FileStruct { false, -1, file } };
    }

    return { false, FileStruct { false, -1, nullptr } };
}

napi_value Close::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, fileStruct] = ParseJsOperand(env, funcArg[NARG_POS::FIRST]);
    if (!resGetFirstArg) {
        HILOGI("Failed to parse fd or FileEntity from JS parameter");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    if (fileStruct.isFd) {
        auto err = CloseFd(fileStruct.fd);
        if (err) {
            err.ThrowErr(env);
            return nullptr;
        }
    } else {
        auto err = CloseFd(fileStruct.fileEntity->fd_->GetFD());
        if (err) {
            err.ThrowErr(env);
            return nullptr;
        }
        auto fp = NClass::RemoveEntityOfFinal<FileEntity>(env, funcArg[NARG_POS::FIRST]);
        if (!fp) {
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Close::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, fileStruct] = ParseJsOperand(env, funcArg[NARG_POS::FIRST]);
    if (!resGetFirstArg) {
        HILOGI("Failed to parse JS operand");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    if (!fileStruct.isFd) {
        fileStruct.fd = fileStruct.fileEntity->fd_->GetFD();
        fileStruct.isFd = true;
        auto fp = NClass::RemoveEntityOfFinal<FileEntity>(env, funcArg[NARG_POS::FIRST]);
        if (!fp) {
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }

    auto cbExec = [fileStruct = fileStruct]() -> NError {
        if (fileStruct.isFd) {
            return CloseFd(fileStruct.fd);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    size_t argc = funcArg.GetArgc();
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_CLOSE_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_CLOSE_NAME, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS