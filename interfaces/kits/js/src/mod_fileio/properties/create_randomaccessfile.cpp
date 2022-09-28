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
#include "create_randomaccessfile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <uv.h>

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"

#include "../class_randomaccessfile/randomaccessfile_entity.h"
#include "../class_randomaccessfile/randomaccessfile_n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

struct FileInfo {
    bool isPath = false;
    unique_ptr<char[]> path;
    FDGuard fdg;
};

static bool CheckFilePath(napi_env env, string path)
{
    if (access(path.c_str(), 0) == 0) {
        struct stat fileStat;
        int ret = stat(path.c_str(), &fileStat);
        if (ret != 0) {
            UniError(errno).ThrowErr(env, "Cannot stat filepath");
            return false;
        }
        if ((fileStat.st_mode & S_IFMT) != S_IFREG) {
            UniError(EINVAL).ThrowErr(env, "Invalid filepath");
            return false;
        }
    }
    return true;
}

static tuple<bool, FileInfo, size_t> ParseJsFileAndFP(napi_env env, napi_value pathOrFdFromJsArg, napi_value FPFromJs)
{
    auto [succ, fp] = NVal(env, FPFromJs).ToInt32();
    if (succ) {
        auto [isPath, path, ignore] = NVal(env, pathOrFdFromJsArg).ToUTF8String();
        if (isPath) {
            if (CheckFilePath(env, string(path.get()))) {
                return { true, FileInfo { true, move(path), {} }, fp };
            }
            return { false, FileInfo { false, {}, {} }, -1 };
        }
        auto [isFd, fd] = NVal(env, pathOrFdFromJsArg).ToInt32();
        if (isFd) {
            if (fd < 0) {
                UniError(EINVAL).ThrowErr(env, "Invalid fd");
                return { false, FileInfo { false, {}, {} }, -1 };
            }
            return { true, FileInfo { false, {}, { fd, false } }, fp };
        }
        UniError(EINVAL).ThrowErr(env, "The first argument requires filepath/fd");
    }
    UniError(EINVAL).ThrowErr(env, "Invalid fp");
    return { false, FileInfo { false, {}, {} }, -1 };
};

static tuple<bool, int> GetJsFlags(napi_env env, const NFuncArg &funcArg, FileInfo &fileInfo)
{
    int flags = O_RDONLY;
    bool succ = false;
    if (fileInfo.isPath) {
        if (funcArg.GetArgc() >= NARG_CNT::THREE && !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function)) {
            tie(succ, flags) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
            if (!succ) {
                UniError(EINVAL).ThrowErr(env, "Invalid flags");
                return { false, flags };
            }
            (void)CommonFunc::ConvertJsFlags(flags);
        }
    }
    return { true, flags };
}

static NVal InstantiateRandomAccessFile(napi_env env, int fd, size_t fp)
{
    napi_value objRAF = NClass::InstantiateClass(env, RandomAccessFileNExporter::className_, {});
    if (!objRAF) {
        UniError(EIO).ThrowErr(env, "Cannot instantiate randomaccessfile");
        return NVal();
    }

    auto rafEntity = NClass::GetEntityOf<RandomAccessFileEntity>(env, objRAF);
    if (!rafEntity) {
        UniError(EIO).ThrowErr(env, "Cannot instantiate randomaccessfile because of void entity");
        return NVal();
    }
    auto fdg = make_unique<FDGuard>(fd);
    rafEntity->fd_.swap(fdg);
    rafEntity->fpointer = fp;
    return { env, objRAF };
}

napi_value CreateRandomAccessFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto [succ, fileInfo, fp] = ParseJsFileAndFP(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }

    if (fileInfo.isPath) {
        auto [succFlags, flags] = GetJsFlags(env, funcArg, fileInfo);
        if (!succFlags) {
            return nullptr;
        }
        uv_loop_s *loop = nullptr;
        napi_get_uv_event_loop(env, &loop);
        uv_fs_t open_req;
        int ret = uv_fs_open(loop, &open_req, fileInfo.path.get(), flags, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, NULL);
        if (ret < 0) {
            UniError(errno).ThrowErr(env);
            return nullptr;
        }
        fileInfo.fdg.SetFD(open_req.result, false);
        uv_fs_req_cleanup(&open_req);
    }
    return InstantiateRandomAccessFile(env, fileInfo.fdg.GetFD(), fp).val_;
}

struct AsyncCreateRandomAccessFileArg {
    int fd;
    size_t fp;
};

napi_value CreateRandomAccessFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto [succ, fileInfo, fp] = ParseJsFileAndFP(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }
    auto [succFlags, flags] = GetJsFlags(env, funcArg, fileInfo);
    if (!succFlags) {
        return nullptr;
    }
    auto arg = make_shared<AsyncCreateRandomAccessFileArg>();
    auto cbExec = [arg, fileInfo = make_shared<FileInfo>(move(fileInfo)), fp = fp, flags =
        flags](napi_env env) -> UniError {
        if (fileInfo->isPath) {
            uv_loop_s *loop = nullptr;
            napi_get_uv_event_loop(env, &loop);
            uv_fs_t open_req;
            int ret = uv_fs_open(loop, &open_req, fileInfo->path.get(), flags, S_IRUSR |
                S_IWUSR | S_IRGRP | S_IWGRP, NULL);
            if (ret < 0) {
                return UniError(errno);
            }
            fileInfo->fdg.SetFD(open_req.result, false);
            uv_fs_req_cleanup(&open_req);
        }
        arg->fd = fileInfo->fdg.GetFD();
        arg->fp = fp;
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateRandomAccessFile(env, arg->fd, arg->fp);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(createRAFProcedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::FOUR) ? NARG_POS::FOURTH : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(createRAFProcedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS