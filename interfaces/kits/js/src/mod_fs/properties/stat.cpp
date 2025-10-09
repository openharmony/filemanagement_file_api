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
#include "stat.h"

#include <memory>
#include <tuple>
#include <unistd.h>
#include <securec.h>

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#include "common_func.h"
#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
const std::string SCHEME_FILE = "file";

static tuple<bool, FileInfo> ParseJsFileByPath(napi_env env, std::unique_ptr<char[]> path)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    string pathStr(path.get());
    if (pathStr.find("://") != string::npos) {
        Uri uri(pathStr);
        string uriType = uri.GetScheme();
        if (uriType == SCHEME_FILE) {
            AppFileService::ModuleFileUri::FileUri fileUri(pathStr);
            string realPath = fileUri.GetRealPath();
            auto pathPtr = std::make_unique<char[]>(realPath.length() + 1);
            auto ret = strncpy_s(pathPtr.get(), realPath.length() + 1, realPath.c_str());
            if (ret != EOK) {
                HILOGE("failed to copy file path");
                NError(ENOMEM).ThrowErr(env);
                return { false, FileInfo { false, {}, {} } };
            }
            return { true, FileInfo { true, move(pathPtr), {} } };
        }
        HILOGE("Failed to stat file by invalid uri");
        NError(EINVAL).ThrowErr(env);
        return { false, FileInfo { false, {}, {} } };
    }
#endif
    return { true, FileInfo { true, move(path), {} } };
}

static tuple<bool, FileInfo> ParseJsFile(napi_env env, napi_value pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = NVal(env, pathOrFdFromJsArg).ToUTF8StringPath();
    if (isPath) {
        return ParseJsFileByPath(env, std::move(path));
    }
    auto [isFd, fd] = NVal(env, pathOrFdFromJsArg).ToInt32();
    if (isFd) {
        if (fd < 0) {
            HILOGE("Invalid fd");
            NError(EINVAL).ThrowErr(env);
            return { false, FileInfo { false, {}, {} } };
        }
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
        if (fdg == nullptr) {
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

static NError CheckFsStat(const FileInfo &fileInfo, uv_fs_t* req)
{
    FileFsTrace traceCheckFsStat("CheckFsStat");
    if (fileInfo.isPath) {
        int ret = uv_fs_stat(nullptr, req, fileInfo.path.get(), nullptr);
        if (ret < 0) {
            HILOGD("Failed to stat file with path, ret is %{public}d", ret);
            if (FileApiDebug::isLogEnabled) {
                HILOGD("Path is %{public}s", fileInfo.path.get());
            }
            return NError(ret);
        }
    } else {
        int ret = uv_fs_fstat(nullptr, req, fileInfo.fdg->GetFD(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to stat file with fd, ret is %{public}d", ret);
            return NError(ret);
        }
    }
    return NError(ERRNO_NOERR);
}

napi_value Stat::Sync(napi_env env, napi_callback_info info)
{
    FileFsTrace traceStatSync("StatSync");
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

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto err = CheckFsStat(fileInfo, stat_req.get());
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto arg = CreateSharedPtr<FileInfo>(move(fileInfo));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto stat = CommonFunc::InstantiateStat(env, stat_req->statbuf, arg).val_;
#else
    auto stat = CommonFunc::InstantiateStat(env, stat_req->statbuf).val_;
#endif
    return stat;
}

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
    auto arg = CreateSharedPtr<StatEntity>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, fileInfo = make_shared<FileInfo>(move(fileInfo))]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
            new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!stat_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        auto err = CheckFsStat(*fileInfo, stat_req.get());
        if (err) {
            return err;
        }
        arg->stat_ = stat_req->statbuf;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        arg->fileInfo_ = fileInfo;
#endif
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        return CommonFunc::InstantiateStat(env, arg->stat_, arg->fileInfo_, true);
#else
        return CommonFunc::InstantiateStat(env, arg->stat_, true);
#endif
    };

    NVal thisVar(env, funcArg.GetThisVar());
    NVal callbackVal(env, funcArg[SECOND]);
    return (funcArg.GetArgc() == NARG_CNT::ONE) ?
        NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_ :
        NAsyncWorkCallback(env, thisVar, callbackVal).Schedule(PROCEDURE_STAT_NAME, cbExec, cbCompl).val_;
}
} // namespace OHOS::FileManagement::ModuleFileIO