/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "disconnectdfs.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace fs = std::filesystem;

napi_value DisconnectDfs::Async(napi_env env, napi_callback_info info)
{
    HILOGI("DisconnectDfs::Async called");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succNetworkId, networkId] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    if (!succNetworkId) {
        HILOGE("Failed to get networkId arguments");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [networkId = move(networkId)]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> disconnectdfs_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!disconnectdfs_req) {
            HILOGE("Failed to request heap memory.");
            return NError(E_PARAMS);
        }
        auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
            CloseP2PConnectionEx(networkId);
        if (ret != ERRNO_NOERR) {
            HILOGE("Failed to CloseP2PConnection, ret = %{public}d", ret);
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };
    
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_DISCONNECTDFS_NAME, cbExec, cbComplete).val_;
}

tuple<bool, std::string> DisconnectDfs::ParseJsOperand(napi_env env, NVal paramFromJsArg)
{
    auto [succ, param, ignore] = paramFromJsArg.ToUTF8String();
    if (!succ) {
        HILOGE("parse parameter failed.");
        return { false, "" };
    }
    std::string paramStr = std::string(param.get());
    return { true, paramStr };
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS