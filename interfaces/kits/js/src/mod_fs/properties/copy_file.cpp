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

#include "copy_file.h"

#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static NError IsAllPath(FileInfo& srcFile, FileInfo& destFile)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> copyfile_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!copyfile_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_copyfile(nullptr, copyfile_req.get(), srcFile.path.get(), destFile.path.get(),
                             UV_FS_COPYFILE_FICLONE, nullptr);
    if (ret < 0) {
        HILOGE("Failed to copy file when all parameters are paths");
        return NError(errno);
    }
    return NError(ERRNO_NOERR);
}

static NError SendFileCore(FileInfo& srcFdg, FileInfo& destFdg, struct stat& statbf)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> sendfile_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!sendfile_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_sendfile(nullptr, sendfile_req.get(), destFdg.fdg->GetFD(), srcFdg.fdg->GetFD(), 0,
                             statbf.st_size, nullptr);
    if (ret < 0) {
        HILOGE("Failed to sendfile by ret : %{public}d", ret);
        return NError(errno);
    }
    return NError(ERRNO_NOERR);
}

static NError OpenFile(FileInfo& srcFile, FileInfo& destFile)
{
    if (srcFile.isPath) {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), srcFile.path.get(), O_RDWR,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open srcFile with ret: %{public}d", ret);
            return NError(errno);
        }
        try {
            srcFile.fdg = make_unique<DistributedFS::FDGuard>(ret, true);
        } catch (const bad_alloc &) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
    }

    struct stat statbf;
    if (fstat(srcFile.fdg->GetFD(), &statbf) < 0) {
        HILOGE("Failed to get stat of file by fd: %{public}d", srcFile.fdg->GetFD());
        return NError(errno);
    }

    if (destFile.isPath) {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), destFile.path.get(), O_RDWR | O_CREAT, statbf.st_mode, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open destFile with ret: %{public}d", ret);
            return NError(errno);
        }
        try {
            destFile.fdg = make_unique<DistributedFS::FDGuard>(ret, true);
        } catch (const bad_alloc &) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
    }
    return SendFileCore(srcFile, destFile, statbf);
}

static tuple<bool, int> ParseJsMode(napi_env env, const NFuncArg& funcArg)
{
    bool succ = false;
    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!succ || mode) {
            return { false, mode };
        }
    }
    return { true, mode };
}

static tuple<bool, FileInfo> ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = pathOrFdFromJsArg.ToUTF8String();
    if (isPath) {
        return { true, FileInfo { true, move(path), {} } };
    }

    auto [isFd, fd] = pathOrFdFromJsArg.ToInt32();
    if (isFd) {
        unique_ptr<DistributedFS::FDGuard> fdg;
        try {
            fdg = make_unique<DistributedFS::FDGuard>(fd, false);
        } catch (const bad_alloc &) {
            HILOGE("Failed to request heap memory.");
            NError(ENOMEM).ThrowErr(env);
            return { false, FileInfo { false, {}, {} } };    return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, move(fdg) } };
    }

    return { false, FileInfo { false, {}, {} } };
};

napi_value CopyFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    auto [succDest, dest] = ParseJsOperand(env, { env, funcArg[NARG_POS::SECOND] });
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath/fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succMode, mode] = ParseJsMode(env, funcArg);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    if (src.isPath && dest.isPath) {
        auto err = IsAllPath(src, dest);
        if (err) {
            err.ThrowErr(env);
            return nullptr;
        }
    } else {
        auto err = OpenFile(src, dest);
        if (err) {
            err.ThrowErr(env);
            return nullptr;
        }
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value CopyFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    auto [succDest, dest] = ParseJsOperand(env, { env, funcArg[NARG_POS::SECOND] });
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath/fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succMode, mode] = ParseJsMode(env, funcArg);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    shared_ptr<Para> para;
    try {
        para = make_shared<Para>(move(src), move(dest));
    } catch (const bad_alloc &) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [para]() -> NError {
        if (para->src_.isPath && para->dest_.isPath) {
            return IsAllPath(para->src_, para->dest_);
        }
        return OpenFile(para->src_, para->dest_);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_COPYFILE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_COPYFILE_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS