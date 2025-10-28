/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "stat_core.h"

#include <memory>
#include <securec.h>
#include <tuple>
#include <unistd.h>

#include "file_fs_trace.h"
#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_stat_entity.h"
#include "stat_instantiator.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
const std::string SCHEME_FILE = "file";

static bool IsPath(FileInfo &fileInfo)
{
    auto path = fileInfo.path.get();
    if (path == nullptr || strlen(path) == 0) {
        return false;
    }
    return true;
}

static tuple<bool, int32_t> IsFd(FileInfo &fileInfo)
{
    auto fdg = fileInfo.fdg.get();
    if (fdg == nullptr) {
        return make_tuple(false, 0);
    }
    return make_tuple(true, fdg->GetFD());
}

static tuple<bool, FileInfo> ProcessPath(FileInfo &fileInfo)
{
    auto &path = fileInfo.path;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    std::string pathStr(path.get());
    if (pathStr.find("://") != std::string::npos) {
        Uri uri(pathStr);
        std::string uriType = uri.GetScheme();
        if (uriType == SCHEME_FILE) {
            AppFileService::ModuleFileUri::FileUri fileUri(pathStr);
            std::string realPath = fileUri.GetRealPath();
            auto pathPtr = std::make_unique<char[]>(realPath.length() + 1);
            size_t length = realPath.length() + 1;
            auto ret = strncpy_s(pathPtr.get(), length, realPath.c_str(), length - 1);
            if (ret != EOK) {
                HILOGE("failed to copy file path");
                return { false, FileInfo { false, {}, {} } };
            }
            return { true, FileInfo { true, std::move(pathPtr), {} } };
        }
    }
#endif
    return { true, FileInfo { true, std::move(path), {} } };
}

static tuple<bool, FileInfo> ValidFileInfo(FileInfo &fileInfo)
{
    auto isPath = IsPath(fileInfo);
    if (isPath) {
        return ProcessPath(fileInfo);
    }
    auto [isFd, fd] = IsFd(fileInfo);
    if (isFd) {
        if (fd < 0) {
            HILOGE("Invalid fd");
            return { false, FileInfo { false, {}, {} } };
        }
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
        if (fdg == nullptr) {
            HILOGE("Failed to request heap memory.");
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, move(fdg) } };
    }
    HILOGE("Invalid parameter");
    return { false, FileInfo { false, {}, {} } };
};

static int32_t CheckFsStat(FileInfo &fileInfo, uv_fs_t *req)
{
    FileFsTrace traceCheckFsStat("CheckFsStat");
    if (fileInfo.isPath) {
        int ret = uv_fs_stat(nullptr, req, fileInfo.path.get(), nullptr);
        if (ret < 0) {
            HILOGD("Failed to stat file with path, ret is %{public}d", ret);
            if (FileApiDebug::isLogEnabled) {
                HILOGD("Path is %{public}s", fileInfo.path.get());
            }
            return ret;
        }
    } else {
        int ret = uv_fs_fstat(nullptr, req, fileInfo.fdg->GetFD(), nullptr);
        if (ret < 0) {
            HILOGE("Failed to stat file with fd, ret is %{public}d", ret);
            return ret;
        }
    }
    return ERRNO_NOERR;
}

FsResult<FsStat *> StatCore::DoStat(FileInfo &fileinfo)
{
    FileFsTrace traceDoStat("DoStat");
    auto [succ, info] = ValidFileInfo(fileinfo);
    if (!succ) {
        return FsResult<FsStat *>::Error(EINVAL);
    }

    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> statReq = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!statReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    auto err = CheckFsStat(info, statReq.get());
    if (err) {
        return FsResult<FsStat *>::Error(err);
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto arg = CreateSharedPtr<FileInfo>(move(info));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    auto stat = StatInstantiator::InstantiateStat(statReq->statbuf, arg);
#else
    auto stat = StatInstantiator::InstantiateStat(statReq->statbuf);
#endif
    if (stat == nullptr) {
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    return FsResult<FsStat *>::Success(stat);
}

} // namespace OHOS::FileManagement::ModuleFileIO