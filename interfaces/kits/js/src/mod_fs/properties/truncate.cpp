/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "truncate.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "../common_func.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, FileInfo> ParseJsFile(napi_env env, napi_value pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = NVal(env, pathOrFdFromJsArg).ToUTF8String();
    if (isPath) {
        return { true, FileInfo { true, move(path), {} } };
    }
    auto [isFd, fd] = NVal(env, pathOrFdFromJsArg).ToInt32();
    if (!isFd || fd < 0) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return { false, FileInfo { false, {}, {} } };
    }
    return { true, FileInfo { false, {}, { fd, false } } };
};

napi_value Truncate::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int truncateLen = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64();
        if (!succ) {
            HILOGE("Invalid truncate length");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
    if (fileInfo.isPath) {
        std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*> open_req = { new uv_fs_t, uv_fs_req_cleanup };
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), O_RDWR,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, nullptr);
        if (ret < 0) {
            NError(errno).ThrowErr(env);
            return nullptr;
        }
        std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*> ftruncate_req = { new uv_fs_t, uv_fs_req_cleanup };
        ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), open_req.get()->result, truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by path");
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    } else {
        std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*> ftruncate_req = { new uv_fs_t, uv_fs_req_cleanup };
        int ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), fileInfo.fdg.GetFD(), truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by fd");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value Truncate::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }
    int truncateLen = 0;
    if (funcArg.GetArgc() > NARG_CNT::ONE && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64();
        if (!succ) {
            HILOGE("Invalid truncate length");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
    auto cbExec = [fileInfo = make_shared<FileInfo>(move(fileInfo)), truncateLen, env = env]() -> NError {
        using uv_fs_unique_ptr_type = std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*>;
        if (fileInfo->isPath) {
            uv_fs_unique_ptr_type open_req = { new uv_fs_t, uv_fs_req_cleanup };
            int ret = uv_fs_open(uv_default_loop(), open_req.get(), fileInfo->path.get(), O_RDWR,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, nullptr);
            if (ret < 0) {
                return NError(errno);
            }
            uv_fs_unique_ptr_type ftruncate_req = { new uv_fs_t, uv_fs_req_cleanup };
            ret = uv_fs_ftruncate(uv_default_loop(), ftruncate_req.get(), open_req.get()->result, truncateLen, nullptr);
            if (ret < 0) {
                HILOGE("Failed to truncate file by path");
                return NError(errno);
            }
        } else {
            uv_fs_unique_ptr_type ftruncate_req = { new uv_fs_t, uv_fs_req_cleanup };
            int ret = uv_fs_ftruncate(uv_default_loop(), ftruncate_req.get(), fileInfo->fdg.GetFD(), truncateLen, nullptr);
            if (ret < 0) {
                HILOGE("Failed to truncate file by fd");
                return NError(errno);
            }
        }
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
    if (funcArg.GetArgc() == NARG_CNT::ONE ||
        (funcArg.GetArgc() == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
    } else {
        if (funcArg.GetArgc() == NARG_CNT::ONE) {
            NVal cb(env, funcArg[NARG_POS::SECOND]);
            return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
        } else {
            NVal cb(env, funcArg[NARG_POS::THIRD]);
            return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
        }
    }
}    
} // namespace OHOS::FileManagement::ModuleFileIO