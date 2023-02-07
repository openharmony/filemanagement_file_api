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
#include "stat_v9.h"

#include <memory>
#include <tuple>

#include "../../common/file_helper/fd_guard.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"

#include "../class_stat_v9/stat_entity_v9.h"
#include "../class_stat_v9/stat_n_exporter_v9.h"

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

napi_value StatV9::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }

    struct stat buf;
    if (fileInfo.isPath) {
        if (stat(fileInfo.path.get(), &buf) != 0) {
            UniError(errno, true).ThrowErr(env);
            return nullptr;
        }
    } else {
        if (fstat(fileInfo.fdg.GetFD(), &buf) != 0) {
            UniError(errno, true).ThrowErr(env);
            return nullptr;
        }
    }

    napi_value objStat = NClass::InstantiateClass(env, StatNExporterV9::className_, {});
    if (!objStat) {
        UniError(EIO, true).ThrowErr(env);
        return nullptr;
    }
    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, objStat);
    if (!statEntity) {
        UniError(EIO, true).ThrowErr(env);
        return nullptr;
    }
    statEntity->stat_ = buf;
    return objStat;
}

struct AsyncStatArg {
    struct stat stat_;
};

napi_value StatV9::Async(napi_env env, napi_callback_info info)
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

    auto arg = make_shared<AsyncStatArg>();
    auto cbExec = [arg, fileInfo = make_shared<FileInfo>(move(fileInfo))](napi_env env) -> UniError {
        if (fileInfo->isPath) {
            if (stat(fileInfo->path.get(), &arg->stat_) != 0) {
                return UniError(errno, true);
            }
        } else {
            if (fstat(fileInfo->fdg.GetFD(), &arg->stat_) != 0) {
                return UniError(errno, true);
            }
        }
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value objStat = NClass::InstantiateClass(env, StatNExporterV9::className_, {});
        if (!objStat) {
            return { env, UniError(EIO, true).GetNapiErr(env) };
        }
        auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, objStat);
        if (!statEntity) {
            return { env, UniError(EIO, true).GetNapiErr(env) };
        }
        statEntity->stat_ = arg->stat_;
        return { env, objStat };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS