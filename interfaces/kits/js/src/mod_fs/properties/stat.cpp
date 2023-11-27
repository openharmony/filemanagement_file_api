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
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <string>
#include <sys/xattr.h>
#endif
#include <tuple>

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#include "common_func.h"
#include "file_utils.h"
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
    if (fileInfo.isPath) {
        int ret = uv_fs_stat(nullptr, req, fileInfo.path.get(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to stat file with path");
            return NError(ret);
        }
    } else {
        int ret = uv_fs_fstat(nullptr, req, fileInfo.fdg->GetFD(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to stat file with fd");
            return NError(ret);
        }
    }
    return NError(ERRNO_NOERR);
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
static Location GetLocationAttr(const FileInfo &fileInfo)
{
    const std::string cloudLocationXattr = "user.cloud.location";
    const Location defaultLocation = LOCAL;
    ssize_t size = 0;
    if (fileInfo.isPath) {
        size = getxattr(fileInfo.path.get(), cloudLocationXattr.c_str(), nullptr, 0);
    } else {
        size = fgetxattr(fileInfo.fdg->GetFD(), cloudLocationXattr.c_str(), nullptr, 0);
    }
    if (size <= 0) {
        HILOGE("Getxattr size failed, errno is %{public}d", errno);
        return defaultLocation;
    }

    std::unique_ptr<char[]> value = CreateUniquePtr<char[]>(size + 1);
    if (value == nullptr) {
        HILOGE("Getxattr memory out, errno is %{public}d", errno);
        return defaultLocation;
    }

    if (fileInfo.isPath) {
        size = getxattr(fileInfo.path.get(), cloudLocationXattr.c_str(), value.get(), size);
    } else {
        size = fgetxattr(fileInfo.fdg->GetFD(), cloudLocationXattr.c_str(), value.get(), size);
    }
    if (size <= 0) {
        HILOGE("Getxattr value failed, errno is %{public}d", errno);
        return defaultLocation;
    }

    std::string location = value.get();
    if (!std::all_of(location.begin(), location.end(), ::isdigit)) {
        HILOGE("Getxattr location is not all digit!");
        return defaultLocation;
    }
    return static_cast<Location>(atoi(location.c_str()));
}
#endif

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
    Location location = GetLocationAttr(fileInfo);
    auto stat = CommonFunc::InstantiateStat(env, stat_req->statbuf, location).val_;
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
        arg->location = GetLocationAttr(*fileInfo);
#endif
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        auto stat = CommonFunc::InstantiateStat(env, arg->stat_, arg->location);
#else
        auto stat = CommonFunc::InstantiateStat(env, arg->stat_);
#endif
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