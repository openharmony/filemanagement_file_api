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

#include "read_dir.h"

#include <dirent.h>
#include <memory>
#include <string>
#include <tuple>

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, unique_ptr<char[]>> ParseJsPath(napi_env env, napi_value pathFromJs)
{
    auto [succ, path, ignore] = NVal(env, pathFromJs).ToUTF8String();
    return {succ, move(path)};
}

static bool VerifyFilePath(char* path)
{
    return strcmp(path, ".") != 0 && strcmp(path, "..") != 0;
}

napi_value ReadDir::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        return nullptr;
    }
    auto [succ, path] = ParseJsPath(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }
    unique_ptr<DIR, function<void(DIR *)>> dir = { opendir(path.get()), closedir };
    if (!dir) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    vector<string> dirFiles;
    struct dirent* entry = readdir(dir.get());
    while (entry) {
        if (VerifyFilePath(entry->d_name)) {
            dirFiles.push_back(entry->d_name);
        }
        entry = readdir(dir.get());
    }
    return NVal::CreateArrayString(env, dirFiles).val_;
}

struct ReadDirArgs {
    vector<string> dirFiles;
    explicit ReadDirArgs()
    {
        dirFiles = vector<string>();
    }
    ~ReadDirArgs() = default;
};

napi_value ReadDir::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    string path;
    auto [succ, tmp] = ParseJsPath(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }
    path = tmp.get();
    auto arg = make_shared<ReadDirArgs>();
    auto cbExec = [arg, path](napi_env env) -> UniError {
        unique_ptr<DIR, function<void(DIR *)>> dir = { opendir(path.c_str()), closedir };
        if (!dir) {
            return UniError(errno);
        }
        struct dirent* entry = readdir(dir.get());
        vector<string> dirnames;
        while (entry) {
            if (VerifyFilePath(entry->d_name)) {
                dirnames.push_back(entry->d_name);
            }
            entry = readdir(dir.get());
        }
        arg->dirFiles = dirnames;
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateArrayString(env, arg->dirFiles);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(readdirProcedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(readdirProcedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS