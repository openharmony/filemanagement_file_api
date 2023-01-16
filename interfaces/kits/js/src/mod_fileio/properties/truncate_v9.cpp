/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "truncate_v9.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "../../common/file_helper/fd_guard.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

struct FileInfo {
    bool isPath = false;
    unique_ptr<char[]> path;
    FDGuard fdg;
};

static tuple<bool, FileInfo> ParseJsFile(napi_env env, napi_value pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = NVal(env, pathOrFdFromJsArg).ToUTF8String();
    if (isPath) {
        return { true, FileInfo { true, move(path), {} } };
    }
    auto [isFd, fd] = NVal(env, pathOrFdFromJsArg).ToInt32();
    if (isFd) {
        if (fd < 0) {
            UniError(EINVAL, true).ThrowErr(env);
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, { fd, false } } };
    }
    UniError(EINVAL, true).ThrowErr(env);
    return { false, FileInfo { false, {}, {} } };
};

napi_value TruncateV9::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }
    int truncateLen = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL, true).ThrowErr(env);
            return nullptr;
        }
    }
    if (fileInfo.isPath) {
        if (truncate(fileInfo.path.get(), truncateLen) != 0) {
            UniError(errno, true).ThrowErr(env);
            return nullptr;
        }
    } else {
        if (ftruncate(fileInfo.fdg.GetFD(), truncateLen) != 0) {
            UniError(errno, true).ThrowErr(env);
            return nullptr;
        }
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value TruncateV9::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }
    int truncateLen = 0;
    if (funcArg.GetArgc() > NARG_CNT::ONE && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL, true).ThrowErr(env);
            return nullptr;
        }
    }
    auto cbExec = [fileInfo = make_shared<FileInfo>(move(fileInfo)), truncateLen](napi_env env) -> UniError {
        if (fileInfo->isPath) {
            if (truncate(fileInfo->path.get(), truncateLen) != 0) {
                return UniError(errno, true);
            }
        } else {
            if (ftruncate(fileInfo->fdg.GetFD(), truncateLen) != 0) {
                return UniError(errno, true);
            }
        }
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO && NVal(env,
        funcArg[NARG_POS::SECOND]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS