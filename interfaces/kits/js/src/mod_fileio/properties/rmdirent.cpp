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

#include "rmdirent.h"

#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, unique_ptr<char[]>> ParseJsPath(napi_env env, napi_value pathFromJs)
{
    auto [succ, path, ignore] = NVal(env, pathFromJs).ToUTF8String();
    return {succ, move(path)};
}

static UniError rmdirent(napi_env env, string path)
{
    if (rmdir(path.c_str()) == 0) {
        return UniError(ERRNO_NOERR);
    }
    auto dir = opendir(path.c_str());
    if (!dir) {
        return UniError(errno);
    }
    struct dirent* entry = readdir(dir);
    while (entry) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            entry = readdir(dir);
            continue;
        }
        struct stat fileInformation;
        string filePath = path + '/';
        filePath.insert(filePath.length(), entry->d_name);
        if (stat(filePath.c_str(), &fileInformation) != 0) {
            closedir(dir);
            return UniError(errno);
        }
        if ((fileInformation.st_mode & S_IFMT) == S_IFDIR) {
            auto err = rmdirent(env, filePath);
            if (err) {
                closedir(dir);
                return err;
            }
        } else {
            if (unlink(filePath.c_str()) != 0) {
                closedir(dir);
                return UniError(errno);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);
    if (rmdir(path.c_str()) != 0) {
        return UniError(errno);
    }
    return UniError(ERRNO_NOERR);
}

napi_value Rmdirent::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path] = ParseJsPath(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto err = rmdirent(env, string(path.get()));
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value Rmdirent::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path] = ParseJsPath(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto cbExec = [path = string(path.get())](napi_env env) -> UniError {
        return rmdirent(env, path);
    };
    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };
    
    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(rmdirProcedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(rmdirProcedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS