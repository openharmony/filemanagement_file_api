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
#include "create_randomaccessfile.h"

#include "class_file/file_entity.h"
#include "class_randomaccessfile/randomaccessfile_entity.h"
#include "class_randomaccessfile/randomaccessfile_n_exporter.h"
#include "common_func.h"
#include "file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

struct RandomAccessFileOps {
    int64_t fp;
    int64_t start;
    int64_t end;
};

static FileEntity* GetFileEntity(napi_env env, napi_value objFile)
{
    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return nullptr;
    }
    if (!fileEntity->fd_) {
        HILOGE("The fd of entity is not exist");
        return nullptr;
    }
    return fileEntity;
}

static tuple<bool, FileInfo, int> ParseJsFile(napi_env env, napi_value pathOrFileFromJsArg)
{
    auto [isPath, path, ignore] = NVal(env, pathOrFileFromJsArg).ToUTF8StringPath();
    if (isPath) {
        OHOS::DistributedFS::FDGuard sfd;
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(sfd, false);
        if (fdg == nullptr) {
            HILOGE("Failed to request heap memory.");
            close(sfd);
            return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
        }
        return { true, FileInfo { true, move(path), move(fdg) }, ERRNO_NOERR};
    }
    auto fileEntity = GetFileEntity(env, pathOrFileFromJsArg);
    if (fileEntity) {
        auto fd = fileEntity->fd_.get()->GetFD();
        if (fd < 0) {
            HILOGE("Invalid fd");
            return { false, FileInfo { false, nullptr, nullptr }, EINVAL};
        }
        auto dupFd = dup(fd);
        if (dupFd < 0) {
            HILOGE("Failed to get valid fd, fail reason: %{public}s, fd: %{public}d", strerror(errno), fd);
            return { false, FileInfo { false, nullptr, nullptr }, EINVAL};
        }
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(dupFd, false);
        if (fdg == nullptr) {
            HILOGE("Failed to request heap memory.");
            close(dupFd);
            return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
        }
        return { true, FileInfo { false, nullptr, move(fdg) }, ERRNO_NOERR};
    }
    HILOGE("The first argument requires filepath/file");
    return { false, FileInfo { false, nullptr, nullptr }, EINVAL};
}

static tuple<bool, int64_t, int64_t> GetRafOptions(napi_env env, napi_value options)
{
    NVal op = NVal(env, options);
    int64_t opStart = INVALID_POS;
    int64_t opEnd = INVALID_POS;
    if (op.HasProp("start")) {
        auto [succ, start] = op.GetProp("start").ToInt64();
        if (!succ || start < 0) {
            NError(EINVAL).ThrowErr(env, "Invalid option.start, positive integer is desired");
            return {false, opStart, opEnd};
        }
        opStart = start;
    }
    if (op.HasProp("end")) {
        auto [succ, end] = op.GetProp("end").ToInt64();
        if (!succ || end < 0) {
            NError(EINVAL).ThrowErr(env, "Invalid option.end, positive integer is desired");
            return {false, opStart, opEnd};
        }
        opEnd = end;
    }
    return {true, opStart, opEnd};
}

static tuple<bool, unsigned int, int64_t, int64_t> GetJsFlags(napi_env env, const NFuncArg &funcArg, FileInfo &fileInfo)
{
    unsigned int flags = O_RDONLY;
    int64_t start = INVALID_POS;
    int64_t end = INVALID_POS;
    if (fileInfo.isPath && funcArg.GetArgc() >= NARG_CNT::TWO) {
        auto [succ, mode] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(0);
        if (!succ || mode < 0) {
            HILOGE("Invalid flags");
            NError(EINVAL).ThrowErr(env);
            return {false, flags, start, end};
        }
        flags = static_cast<unsigned int>(mode);
        (void)CommonFunc::ConvertJsFlags(flags);
    }

    bool succOpt;
    if (funcArg.GetArgc() == NARG_CNT::THREE && !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function)) {
        tie(succOpt, start, end) = GetRafOptions(env, funcArg[NARG_POS::THIRD]);
        if (!succOpt) {
            HILOGE("invalid RandomAccessFile options");
            NError(EINVAL).ThrowErr(env);
            return {false, flags, start, end};
        }
    }

    return {true, flags, start, end};
}

static NVal InstantiateRandomAccessFile(napi_env env,
                                        std::unique_ptr<DistributedFS::FDGuard> fdg,
                                        struct RandomAccessFileOps ops,
                                        bool async = false)
{
    napi_value objRAF = NClass::InstantiateClass(env, RandomAccessFileNExporter::className_, {});
    if (!objRAF) {
        HILOGE("Cannot instantiate randomaccessfile");
        if (async) {
            return {env, NError(EIO).GetNapiErr(env)};
        }
        NError(EIO).ThrowErr(env);
        return NVal();
    }
    auto rafEntity = NClass::GetEntityOf<RandomAccessFileEntity>(env, objRAF);
    if (!rafEntity) {
        HILOGE("Cannot instantiate randomaccessfile because of void entity");
        if (async) {
            return {env, NError(EIO).GetNapiErr(env)};
        }
        NError(EIO).ThrowErr(env);
        return NVal();
    }
    rafEntity->fd.swap(fdg);
    rafEntity->filePointer = ops.fp;
    rafEntity->start = ops.start;
    rafEntity->end = ops.end;
    return {env, objRAF};
}

napi_value CreateRandomAccessFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo, err] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        NError(err).ThrowErr(env);
        return nullptr;
    }
    if (fileInfo.isPath) {
        auto [succFlags, flags, ignoreStart, ignoreEnd] = GetJsFlags(env, funcArg, fileInfo);
        if (!succFlags) {
            return nullptr;
        }
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            NError(ENOMEM).ThrowErr(env);
            return nullptr;
        }
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), flags, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, NULL);
        if (ret < 0) {
            NError(ret).ThrowErr(env);
            return nullptr;
        }

        fileInfo.fdg->SetFD(open_req.get()->result, false);
    }
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        auto [succ, start, end] = GetRafOptions(env, funcArg[NARG_POS::THIRD]);
        if (succ) {
            return InstantiateRandomAccessFile(env, move(fileInfo.fdg), {0, start, end}).val_;
        }
    }
    return InstantiateRandomAccessFile(env, move(fileInfo.fdg), {0, INVALID_POS, INVALID_POS}).val_;
}

struct AsyncCreateRandomAccessFileArg {
    int fd = 0;
};

NError AsyncExec(shared_ptr<AsyncCreateRandomAccessFileArg> arg,
    shared_ptr<FileInfo> fileInfo, unsigned int flags)
{
    if (fileInfo->isPath) {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new (nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo->path.get(), flags, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, NULL);
        if (ret < 0) {
            return NError(ret);
        }
        fileInfo->fdg->SetFD(open_req.get()->result, false);
    }
    arg->fd = fileInfo->fdg->GetFD();
    return NError(ERRNO_NOERR);
}

napi_value CreateRandomAccessFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo, err] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        NError(err).ThrowErr(env);
        return nullptr;
    }
    auto [succFlags, flags, start, end] = GetJsFlags(env, funcArg, fileInfo);
    if (!succFlags) {
        return nullptr;
    }
    auto arg = CreateSharedPtr<AsyncCreateRandomAccessFileArg>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto movedFileInfo = CreateSharedPtr<FileInfo>(move(fileInfo));
    if (movedFileInfo == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, movedFileInfo, flags = flags]() -> NError {
        return AsyncExec(arg, movedFileInfo, flags);
    };

    auto cbCompl = [arg, movedFileInfo, start = start, end = end](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateRandomAccessFile(env, move(movedFileInfo->fdg), {0, start, end}, true);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE ||
        (funcArg.GetArgc() == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) ||
        (funcArg.GetArgc() == NARG_CNT::THREE && NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_object))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_CREATERAT_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb, PROCEDURE_CREATERAT_NAME)
            .Schedule(PROCEDURE_CREATERAT_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS