/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "listfile_ext_napi.h"

#include <memory>
#include <string>

#include "file_filter_napi.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "listfile_ext_core.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;

static std::optional<NVal> GetFilterException(napi_env env, const std::optional<ListFileExtOptions> &options)
{
    if (!options.has_value()) {
        return nullopt;
    }
    auto napiFilter = std::static_pointer_cast<FileFilterNapi>(options.value().fileFilter);
    if (!napiFilter || !napiFilter->HasException()) {
        return nullopt;
    }
    return NVal(env, napiFilter->HandleException(env));
}

static bool GetFileFilterFunction(const NVal &fileFilterProp, ListFileExtOptions &options)
{
    auto filterCallback = fileFilterProp.GetProp("filter");
    if (!filterCallback.TypeIs(napi_function)) {
        HILOGE("Failed to get FileFilter.filter function");
        return false;
    }
    options.fileFilter = CreateSharedPtr<FileFilterNapi>(fileFilterProp.env_, fileFilterProp.val_, filterCallback);
    if (!options.fileFilter) {
        HILOGE("Failed to request heap memory.");
        return false;
    }
    return true;
}

static bool GetOptionParam(const NVal &argv, ListFileExtOptions &options)
{
    bool succ = false;
    if (argv.HasProp("listNum")) {
        tie(succ, options.listNum) = argv.GetProp("listNum").ToInt64(0);
        if (!succ) {
            HILOGE("Failed to get listNum prop");
            return false;
        }
    }

    if (argv.HasProp("recursion")) {
        tie(succ, options.recursion) = argv.GetProp("recursion").ToBool(false);
        if (!succ) {
            HILOGE("Failed to get recursion prop.");
            return false;
        }
    }

    if (argv.HasProp("fileFilter")) {
        NVal fileFilterProp = argv.GetProp("fileFilter");
        if (!fileFilterProp.TypeIs(napi_undefined)) {
            succ = GetFileFilterFunction(fileFilterProp, options);
            if (!succ) {
                HILOGE("Failed to get fileFilter prop.");
                return false;
            }
        }
    }
    return true;
}

static std::tuple<bool, std::optional<ListFileExtOptions>> ParseOptionsArg(napi_env env, const NFuncArg &funcArg)
{
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return { true, nullopt };
    }
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        auto optArg = NVal(env, funcArg[NARG_POS::SECOND]);
        if (optArg.TypeIs(napi_object)) {
            ListFileExtOptions options;
            auto succ = GetOptionParam(optArg, options);
            return { succ, options };
        } else if (optArg.TypeIs(napi_undefined) || optArg.TypeIs(napi_function)) {
            return { true, nullopt };
        }
    }
    return { false, nullopt };
}

napi_value ListFileExtNapi::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, options] = ParseOptionsArg(env, funcArg);
    if (!succ) {
        HILOGE("Invalid options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto ret = ListFileExtCore::DoListFileExt(string(path.get()), options);
    if (!ret.IsSuccess()) {
        NError(ret.GetError().GetErrNo()).ThrowErr(env);
        return nullptr;
    }
    auto direntsRes = ret.GetData().value();
    return NVal::CreateArrayString(env, direntsRes).val_;
}

napi_value ListFileExtNapi::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succOpt, tmpOptions] = ParseOptionsArg(env, funcArg);
    if (!succOpt) {
        HILOGE("Invalid options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    std::optional<ListFileExtOptions> options = tmpOptions;
    auto arg = CreateSharedPtr<ListFileExtArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, options, pathStr = string(path.get())]() -> NError {
        auto ret = ListFileExtCore::DoListFileExt(pathStr, options);
        if (!ret.IsSuccess()) {
            return NError(ret.GetError().GetErrNo());
        }
        arg->dirents = ret.GetData().value();
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg, options](napi_env env, NError err) -> NVal {
        auto filterException = GetFilterException(env, options);
        if (filterException.has_value()) {
            return filterException.value();
        }
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { env, NVal::CreateArrayString(env, arg->dirents).val_ };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(LIST_FILE_EXT_PRODUCE_NAME, cbExec, cbCompl).val_;
}
} // namespace OHOS::FileManagement::ModuleFileIO
