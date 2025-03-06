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

#include "truncate_core.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

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
        auto &path = const_cast<std::unique_ptr<char[]>&>(fileInfo.path);
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

static int Truncate(FileInfo &fileInfo, int64_t truncateLen)
{
    if (fileInfo.isPath) {
        std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> open_req = {
            new uv_fs_t, FsUtils::FsReqCleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return ENOMEM;
        }
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), O_RDWR,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            return ret;
        }
        std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> ftruncate_req = {
            new uv_fs_t, FsUtils::FsReqCleanup };
        if (!ftruncate_req) {
            HILOGE("Failed to request heap memory.");
            return ENOMEM;
        }
        ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), ret, truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by path");
            return ret;
        }
    } else {
        std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> ftruncate_req = {
            new uv_fs_t, FsUtils::FsReqCleanup };
        if (!ftruncate_req) {
            HILOGE("Failed to request heap memory.");
            return ENOMEM;
        }
        int ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), fileInfo.fdg->GetFD(), truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by fd for libuv error %{public}d", ret);
            return ret;
        }
    }
    return ERRNO_NOERR;
}

FsResult<void> TruncateCore::DoTruncate(FileInfo &fileInfo, const std::optional<int64_t> &len)
{
    auto [succ, info] = ValidFileInfo(fileInfo);
    if (!succ) {
        return FsResult<void>::Error(EINVAL);
    }
    int64_t truncateLen = 0;
    if (len.has_value()) {
        truncateLen = len.value();
        if (truncateLen < 0) {
            HILOGE("Invalid truncate length");
            return FsResult<void>::Error(EINVAL);
        }
    }
    
    auto err = Truncate(info, truncateLen);
    if (err) {
        return FsResult<void>::Error(err);
    }

    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::ModuleFileIO