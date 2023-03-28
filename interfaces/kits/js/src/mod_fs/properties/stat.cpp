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
#include "stat.h"

#include <memory>
#include <tuple>

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"

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
    if (isFd) {
        if (fd < 0) {
            HILOGE("Invalid fd");
            NError(EINVAL).ThrowErr(env);
            return { false, FileInfo { false, {}, {} } };
        }
        auto fdg = make_unique<DistributedFS::FDGuard>(fd, false);
        if (!fdg) {
            HILOGE("Failed to request heap memory.");
            NError(ENOMEM).ThrowErr(env);
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, move(fdg) } };
    }
    HILOGE("Invalid parameter");
    NError(EINVAL).ThrowErr(env);
    return { false, FileInfo { false, {}, {} } };
};

napi_value Stat::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }

    struct stat buf;
    if (fileInfo.isPath) {
        if (stat(fileInfo.path.get(), &buf) < 0) {
            HILOGE("Failed to stat file with path");
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    } else {
        if (fstat(fileInfo.fdg->GetFD(), &buf) < 0) {
            HILOGE("Failed to stat file with fd");
            NError(errno).ThrowErr(env);
            return nullptr;
        }
    }

    auto stat = CommonFunc::InstantiateStat(env, buf).val_;
    return stat;
}

struct AsyncStatArg {
    struct stat stat_;
};

napi_value Stat::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<AsyncStatArg>();
    auto cbExec = [arg, fileInfo = make_shared<FileInfo>(move(fileInfo))]() -> NError {
        if (fileInfo->isPath) {
            if (stat(fileInfo->path.get(), &arg->stat_) < 0) {
                HILOGE("Failed to stat file with path");
                return NError(errno);
            }
        } else {
            if (fstat(fileInfo->fdg->GetFD(), &arg->stat_) < 0) {
                HILOGE("Failed to stat file with fd");
                return NError(errno);
            }
        }
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        auto stat = CommonFunc::InstantiateStat(env, arg->stat_);
        return stat;
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO