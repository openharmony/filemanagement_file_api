/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "environment_n_exporter.h"

#include <string>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
using namespace OHOS::FileManagement::LibN;
namespace {
    const std::string STORAGE_DATA_PATH = "/data";
}

napi_value GetStorageDataDir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, STORAGE_DATA_PATH);
    };

    static const std::string PROCEDURE_NAME = "GetStorageDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

int GetUserId()
{
    return 0;
}

napi_value GetUserDataDir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto userDataPath = std::make_shared<std::string>();
    auto cbExec = [userDataPath]() -> NError {
        (*userDataPath).append("/storage/media/").append(std::to_string(GetUserId())).append("/local");
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [userDataPath](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, *userDataPath);
    };

    static const std::string PROCEDURE_NAME = "GetUserDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS