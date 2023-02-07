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

#include "rmdirent.h"

#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

#ifdef __MUSL__
static NError RmDirent(const string &fpath)
{
    std::filesystem::path strToPath(fpath);
    std::uintmax_t num = std::filesystem::remove_all(strToPath);
    if (!num || std::filesystem::exists(strToPath)) {
        HILOGE("Failed to remove file or directory by path");
        return NError(errno);
    }

    return NError(ERRNO_NOERR);
}

#else
static NError RmDirent(const string &fpath)
{
    if (rmdir(fpath.c_str()) == 0) {
        return NError(ERRNO_NOERR);
    }
    auto dir = opendir(fpath.c_str());
    if (!dir) {
        return NError(errno);
    }
    struct dirent* entry = readdir(dir);
    while (entry) {
        if (strncmp(entry->d_name, ".", strlen(".")) == 0 ||
            strncmp(entry->d_name, "..", strlen("..")) == 0) {
            entry = readdir(dir);
            continue;
        }
        struct stat fileInformation;
        string filePath = fpath + '/';
        filePath.insert(filePath.length(), entry->d_name);
        if (stat(filePath.c_str(), &fileInformation) != 0) {
            closedir(dir);
            HILOGE("Failed to close directory");
            return NError(errno);
        }
        if ((fileInformation.st_mode & S_IFMT) == S_IFDIR) {
            auto err = RmDirent(filePath);
            if (err) {
                closedir(dir);
                return err;
            }
        } else {
            if (unlink(filePath.c_str()) != 0) {
                closedir(dir);
                return NError(errno);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);
    if (rmdir(fpath.c_str()) != 0) {
        return NError(errno);
    }
    return NError(ERRNO_NOERR);
}
#endif

napi_value Rmdirent::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto err = RmDirent(string(path.get()));
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
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [tmpPath = string(path.get())]() -> NError {
        return RmDirent(tmpPath);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_RMDIRENT_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_RMDIRENT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS