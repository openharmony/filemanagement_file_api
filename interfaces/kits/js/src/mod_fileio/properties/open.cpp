/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "open.h"
#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>
#include "remote_uri.h"

#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_func_arg.h"
#include "../common_func.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value Open::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> path = nullptr;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    unsigned int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        auto [succGetFlags, authFlags] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(O_RDONLY);
        if (!succGetFlags || authFlags < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid flags");
            return nullptr;
        }
        flags = static_cast<unsigned int>(authFlags);
        (void)CommonFunc::ConvertJsFlags(flags);
    }

    int fd = -1;
    if (ModuleRemoteUri::RemoteUri::IsRemoteUri(path.get(), fd, flags)) {
        return NVal::CreateInt64(env, fd).val_;
    }

    int32_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    if (funcArg.GetArgc() != NARG_CNT::THREE) {
        size_t flagsFirst { flags };
        if ((flagsFirst & O_CREAT) || (flagsFirst & O_TMPFILE)) {
            UniError(EINVAL).ThrowErr(env, "called with O_CREAT/O_TMPFILE but no mode");
            return nullptr;
        }
    } else {
        tie(succ, mode) = NVal(env, funcArg.GetArg(NARG_POS::THIRD)).ToInt32(mode);
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    fd = open(path.get(), flags, mode);
    if (fd == -1) {
        if (errno == ENAMETOOLONG) {
            UniError(errno).ThrowErr(env, "Filename too long");
            return nullptr;
        }
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, fd).val_;
}

static UniError DoOpenExec(const std::string& path, const unsigned int flags, const int32_t mode,
    shared_ptr<int32_t> arg)
{
    int fd = -1;
    if (!ModuleRemoteUri::RemoteUri::IsRemoteUri(path, fd, flags)) {
        fd = open(path.c_str(), flags, mode);
    }
    *arg = fd;
    if (fd == -1) {
        return UniError(errno);
    } else {
        return UniError(ERRNO_NOERR);
    }
}

static bool ParseFlags(napi_env env, const NFuncArg& funcArg, unsigned int& flags)
{
    auto [succ, authFlags] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(O_RDONLY);
    if (!succ || authFlags < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid flags");
        return false;
    }
    flags = static_cast<unsigned int>(authFlags);
    (void)CommonFunc::ConvertJsFlags(flags);
    return true;
}

napi_value Open::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto [succ, path, unuse] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }
    size_t argc = funcArg.GetArgc();
    unsigned int flags = O_RDONLY;
    if (argc >= NARG_CNT::TWO) {
        if (!ParseFlags(env, funcArg, flags)) {
            return nullptr;
        }
    }
    int32_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    if (argc >= NARG_CNT::THREE) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    auto arg = make_shared<int32_t>();
    auto cbExec = [path = string(path.get()), flags, mode, arg](napi_env env) -> UniError {
        return DoOpenExec(path, flags, mode, arg);
    };
    auto cbComplCallback = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            if (err.GetErrno(ERR_CODE_SYSTEM_POSIX) == ENAMETOOLONG) {
                return {env, err.GetNapiErr(env, "Filename too long")};
            }
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *arg) };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE || (argc == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) || (argc == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOOpen", cbExec, cbComplCallback).val_;
    } else {
        NVal cb(env, funcArg[argc - 1]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOOpen", cbExec, cbComplCallback).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS