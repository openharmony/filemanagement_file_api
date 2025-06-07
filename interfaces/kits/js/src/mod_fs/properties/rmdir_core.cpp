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

#include "rmdir_core.h"

#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <tuple>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

#ifdef __MUSL__
static int32_t RmDirent(const string &fpath)
{
    std::filesystem::path strToPath(fpath);
    std::error_code errCode;
    std::uintmax_t num = std::filesystem::remove_all(strToPath, errCode);
    if (errCode.value() != ERRNO_NOERR) {
        HILOGD("Failed to remove directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    if (!num || std::filesystem::exists(strToPath, errCode)) {
        HILOGE("Failed to remove directory, dirPath does not exist");
        return ENOENT;
    }
    if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

#else
static int32_t RmDirent(const string &fpath)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> scandirReq = {
        new (std::nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (!scandirReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = 0;
    ret = uv_fs_scandir(nullptr, scandirReq.get(), fpath.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to scandir, ret: %{public}d", ret);
        return ret;
    }
    uv_dirent_t dent;
    while (uv_fs_scandir_next(scandirReq.get(), &dent) != UV_EOF) {
        string filePath = fpath + "/" + string(dent.name);
        if (dent.type == UV_DIRENT_FILE) {
            std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> unlinkReq = {
                new (std::nothrow) uv_fs_t, FsUtils::FsReqCleanup };
            if (!unlinkReq) {
                HILOGE("Failed to request heap memory.");
                return ENOMEM;
            }
            ret = uv_fs_unlink(nullptr, unlinkReq.get(), filePath.c_str(), nullptr);
            if (ret < 0) {
                HILOGE("Failed to unlink file, ret: %{public}d", ret);
                return ret;
            }
        } else if (dent.type == UV_DIRENT_DIR) {
            auto rmDirentRes = RmDirent(filePath);
            if (rmDirentRes) {
                return rmDirentRes;
            }
        }
    }
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> rmdirReq = {
        new (std::nothrow) uv_fs_t, FsUtils::FsReqCleanup};
    if (!rmdirReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    ret = uv_fs_rmdir(nullptr, rmdirReq.get(), fpath.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to rmdir empty dir, ret: %{public}d", ret);
        return ret;
    }
    return ERRNO_NOERR;
}
#endif

FsResult<void> RmdirentCore::DoRmdirent(const string &fpath)
{
    if (fpath.empty()) {
        HILOGE("Invalid path");
        return FsResult<void>::Error(EINVAL);
    }

    auto err = RmDirent(fpath);
    if (err) {
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS