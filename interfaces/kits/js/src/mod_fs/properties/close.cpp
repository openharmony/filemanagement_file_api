/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstring>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "file_fs_trace.h"
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
        HILOGE("The fd of entity is not exist");
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
        HILOGE("Failed to uv_fs_close file with ret: %{public}d", ret);
        return NError(ret);
    }
    return NError(ERRNO_NOERR);
}

static NError CloseFdWithFdsan(const int fd, const bool isFd, const uint64_t fileTag)
{
    FileFsTrace traceCloseFd("CloseFd");
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    if (fd >= FD_SAN_OVERFLOW_END) {
        return CloseFd(fd);
    }

    if (isFd) {
        CommonFunc::SetFdTag(fd, 0);
        return CloseFd(fd);
    } else {
        auto tag = CommonFunc::GetFdTag(fd);
        if (tag <= 0 || tag != fileTag) {
            tag = fileTag|PREFIX_ADDR;
        } else {
            tag = 0;
        }
        CommonFunc::SetFdTag(fd, 0);
        int ret = fdsan_close_with_tag(fd, tag);
        if (ret < 0) {
            HILOGE("Failed to close file with errno: %{public}d", errno);
            return NError(errno);
        }
    }
    return NError(ERRNO_NOERR);
#else
    return CloseFd(fd);
#endif
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
    FileFsTrace traceCloseSync("CloseSync");
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

    uint64_t fileTag = static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(fileStruct.fileEntity));
    if (fileStruct.isFd) {
        auto err = CloseFdWithFdsan(fileStruct.fd, fileStruct.isFd, fileTag);
        if (err) {
            err.ThrowErr(env);
            return nullptr;
        }
    } else {
        auto err = CloseFdWithFdsan(fileStruct.fileEntity->fd_->GetFD(), fileStruct.isFd, fileTag);
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

    uint64_t fileTag = static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(fileStruct.fileEntity));

    if (!fileStruct.isFd) {
        fileStruct.fd = fileStruct.fileEntity->fd_->GetFD();
        auto fp = NClass::RemoveEntityOfFinal<FileEntity>(env, funcArg[NARG_POS::FIRST]);
        if (!fp) {
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }

    auto cbExec = [fileStruct = fileStruct, fileTag]() -> NError {
        return CloseFdWithFdsan(fileStruct.fd, fileStruct.isFd, fileTag);
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
        return NAsyncWorkCallback(env, thisVar, cb, PROCEDURE_CLOSE_NAME)
            .Schedule(PROCEDURE_CLOSE_NAME, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS