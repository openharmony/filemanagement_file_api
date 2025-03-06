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
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_stat_entity.h"
#include "stat_instantiator.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static bool IsPath(const FileInfo &fileInfo)
{
    auto path = fileInfo.path.get();
    if (path == nullptr || strlen(path) == 0) {
        return false;
    }
    return true;
}

static tuple<bool, int32_t> IsFd(const FileInfo &fileInfo)
{
    auto fdg = fileInfo.fdg.get();
    if (fdg == nullptr) {
        return make_tuple(false, 0);
    }
    return make_tuple(true, fdg->GetFD());
}

static tuple<bool, FileInfo> ValidFileInfo(const FileInfo &fileInfo)
{
    auto isPath = IsPath(fileInfo);
    if (isPath) {
        auto &path = const_cast<std::unique_ptr<char[]> &>(fileInfo.path);
        return { true, FileInfo { true, move(path), {} } };
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

static int32_t CheckFsStat(const FileInfo &fileInfo, uv_fs_t *req)
{
    if (fileInfo.isPath) {
        int ret = uv_fs_stat(nullptr, req, fileInfo.path.get(), nullptr);
        if (ret < 0) {
            HILOGD("Failed to stat file with path, ret is %{public}d", ret);
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

FsResult<FsStat *> StatCore::DoStat(const FileInfo &fileinfo)
{
    auto [succ, info] = ValidFileInfo(fileinfo);
    if (!succ) {
        return FsResult<FsStat *>::Error(EINVAL);
    }

    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> stat_req = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    auto err = CheckFsStat(info, stat_req.get());
    if (err) {
        return FsResult<FsStat *>::Error(err);
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto arg = CreateSharedPtr<FileInfo>(move(info));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    auto stat = StatInstantiator::InstantiateStat(stat_req->statbuf, arg);
#else
    auto stat = StatInstantiator::InstantiateStat(stat_req->statbuf);
#endif
    if (stat == nullptr) {
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    return FsResult<FsStat *>::Success(stat);
}

} // namespace OHOS::FileManagement::ModuleFileIO