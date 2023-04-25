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

#include "securitylabel_n_exporter.h"

#include <singleton.h>
#include <tuple>

#include "filemgmt_libhilog.h"
#include "security_label.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
using namespace OHOS::FileManagement::LibN;
using namespace std;

napi_value SetSecurityLabel(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO), static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    std::unique_ptr<char []> dataLevel;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, dataLevel, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid dataLevel");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    std::string dataLevelString(dataLevel.get());
    if (DATA_LEVEL.find(dataLevelString) == DATA_LEVEL.end()) {
        HILOGE("Invalid Argument of dataLevelEnum");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [pathString = string(path.get()), dataLevelString]() -> NError {
        bool ret = SecurityLabel::SetSecurityLabel(pathString, dataLevelString);
        if (!ret) {
            return NError(errno);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };
    static const std::string PROCEDURE_NAME = "SetSecurityLabel";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == static_cast<size_t>(NARG_CNT::TWO)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[static_cast<int>(NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }
}

napi_value SetSecurityLabelSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO), static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    std::unique_ptr<char []> dataLevel;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    tie(succ, dataLevel, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid dataLevel");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    if (DATA_LEVEL.find(dataLevel.get()) == DATA_LEVEL.end()) {
        HILOGE("Invalid Argument of dataLevelEnum");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool ret = SecurityLabel::SetSecurityLabel(path.get(), dataLevel.get());
    if (!ret) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value GetSecurityLabel(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::ONE), static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto result = std::make_shared<string>();
    std::string pathString(path.get());
    auto cbExec = [pathString = move(pathString), result]() -> NError {
        *result = SecurityLabel::GetSecurityLabel(pathString);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUTF8String(env, *result) };
    };

    static const std::string PROCEDURE_NAME = "GetSecurityLabel";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == static_cast<size_t>(NARG_CNT::ONE)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[static_cast<int>(NARG_POS::SECOND)]);
        if (cb.TypeIs(napi_function)) {
            return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
        }
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetSecurityLabelSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::ONE), static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    std::string result = SecurityLabel::GetSecurityLabel(path.get());
    return NVal::CreateUTF8String(env, result).val_;
}
} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS