/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "copy_file.h"

#include <cstring>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "../../common/file_helper/fd_guard.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

namespace {
constexpr int COPY_BLOCK_SIZE = 4096;
}

struct FileInfo {
    bool isPath = false;
    unique_ptr<char[]> path;
    FDGuard fdg;
};

static UniError CopyFileCore(FileInfo &srcFile, FileInfo &destFile)
{
    int res = EINVAL;
    if (srcFile.isPath) {
        srcFile.fdg.SetFD(open(srcFile.path.get(), O_RDONLY), true);
        res = errno;
    }

    if (!srcFile.fdg) {
        return UniError(res);
    }

    struct stat statbf;
    if (fstat(srcFile.fdg.GetFD(), &statbf) == -1) {
        return UniError(errno);
    }

    if (destFile.isPath) {
        destFile.fdg.SetFD(open(destFile.path.get(), O_WRONLY | O_CREAT, statbf.st_mode), true);
        res = errno;
    }

    if (!destFile.fdg) {
        return UniError(res);
    }

    auto copyBuf = make_unique<char[]>(COPY_BLOCK_SIZE);
    do {
        ssize_t readSize = read(srcFile.fdg.GetFD(), copyBuf.get(), COPY_BLOCK_SIZE);
        if (readSize == -1) {
            return UniError(errno);
        } else if (readSize == 0) {
            break;
        }
        ssize_t writeSize = write(destFile.fdg.GetFD(), copyBuf.get(), readSize);
        if (writeSize != readSize) {
            return UniError(errno);
        }
        if (readSize != COPY_BLOCK_SIZE) {
            break;
        }
    } while (true);

    return UniError(ERRNO_NOERR);
}

static tuple<bool, int, bool> ParseJsModeAndProm(napi_env env, const NFuncArg &funcArg)
{
    bool succ = false;
    bool promise = false;
    bool hasMode = false;
    int mode = 0;
    if (funcArg.GetArgc() == NARG_CNT::THREE && NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_number)) {
        promise = true;
        hasMode = true;
    } else if (funcArg.GetArgc() == NARG_CNT::FOUR) {
        hasMode = true;
    }

    if (hasMode) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
        if (!succ) {
            return {false, mode, promise};
        }
    }

    return {true, mode, promise};
}

static tuple<bool, FileInfo> ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = pathOrFdFromJsArg.ToUTF8String();
    if (isPath) {
        return {true, FileInfo{true, move(path), {}}};
    }

    auto [isFd, fd] = pathOrFdFromJsArg.ToInt32();
    if (isFd) {
        return {true, FileInfo{false, {}, {fd, false}}};
    }

    return {false, FileInfo{false, {}, {}}};
};

napi_value CopyFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, {env, funcArg[NARG_POS::FIRST]});
    auto [succDest, dest] = ParseJsOperand(env, {env, funcArg[NARG_POS::SECOND]});
    if (!succSrc || !succDest) {
        UniError(EINVAL).ThrowErr(env, "The first/second argument requires filepath/fd");
        return nullptr;
    }

    auto [succMode, mode, unused] = ParseJsModeAndProm(env, funcArg);
    if (!succMode) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    auto err = CopyFileCore(src, dest);
    if (err) {
        if (err.GetErrno(ERR_CODE_SYSTEM_POSIX) == ENAMETOOLONG) {
            UniError(EINVAL).ThrowErr(env, "Filename too long");
            return nullptr;
        }
        err.ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

class Para {
public:
    FileInfo src_;
    FileInfo dest_;

    Para(FileInfo src, FileInfo dest) : src_(move(src)), dest_(move(dest)) {};
};

napi_value CopyFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, {env, funcArg[NARG_POS::FIRST]});
    auto [succDest, dest] = ParseJsOperand(env, {env, funcArg[NARG_POS::SECOND]});
    if (!succSrc || !succDest) {
        UniError(EINVAL).ThrowErr(env, "The first/second argument requires filepath/fd");
        return nullptr;
    }

    auto [succMode, mode, promise] = ParseJsModeAndProm(env, funcArg);
    if (!succMode) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    auto cbExec = [para = make_shared<Para>(move(src), move(dest))](napi_env env) -> UniError {
        return CopyFileCore(para->src_, para->dest_);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            if (err.GetErrno(ERR_CODE_SYSTEM_POSIX) == ENAMETOOLONG) {
                return {env, err.GetNapiErr(env, "Filename too long")};
            }
            return {env, err.GetNapiErr(env)};
        }
        return {NVal::CreateUndefined(env)};
    };
    
    const string procedureName = "FileIOCopyFile";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || promise) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
