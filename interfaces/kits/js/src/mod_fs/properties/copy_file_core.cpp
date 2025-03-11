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

#include "copy_file_core.h"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <tuple>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static int32_t IsAllPath(FileInfo &srcFile, FileInfo &destFile)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    filesystem::path srcPath(string(srcFile.path.get()));
    filesystem::path dstPath(string(destFile.path.get()));
    error_code errCode;
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing,
                               errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
#else
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> copyfile_req = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup};
    if (!copyfile_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_copyfile(nullptr, copyfile_req.get(), srcFile.path.get(), destFile.path.get(),
                             UV_FS_COPYFILE_FICLONE, nullptr);
    if (ret < 0) {
        HILOGE("Failed to copy file when all parameters are paths");
        return ret;
    }
#endif
    return ERRNO_NOERR;
}

static int32_t SendFileCore(FileInfo &srcFdg, FileInfo &destFdg, struct stat &statbf)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> sendfile_req = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup};
    if (!sendfile_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int64_t offset = 0;
    size_t size = static_cast<size_t>(statbf.st_size);
    int ret = 0;
    while (size > 0) {
        ret = uv_fs_sendfile(nullptr, sendfile_req.get(), destFdg.fdg->GetFD(), srcFdg.fdg->GetFD(),
                             offset, std::min(MAX_SIZE, size), nullptr);
        if (ret < 0) {
            HILOGE("Failed to sendfile by ret : %{public}d", ret);
            return ret;
        }
        if (static_cast<size_t>(ret) > size) {
            HILOGE("More bytes returned than the size of the file. The file size is "
                   "%{public}zu"
                   "The bytes returned is %{public}d",
                   size, ret);
            return EIO;
        }
        offset += static_cast<int64_t>(ret);
        size -= static_cast<size_t>(ret);
        if (ret == 0) {
            break;
        }
    }
    if (size != 0) {
        HILOGE("The execution of the sendfile task was terminated, remaining file "
               "size %{public}zu", size);
        return EIO;
    }
    return ERRNO_NOERR;
}

static int32_t TruncateCore(const FileInfo &fileInfo)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> ftruncateReq = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup};
    if (!ftruncateReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_ftruncate(nullptr, ftruncateReq.get(), fileInfo.fdg->GetFD(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to truncate dstFile with ret: %{public}d", ret);
        return ret;
    }
    return ERRNO_NOERR;
}

static int32_t OpenCore(FileInfo &fileInfo, const int flags, const int mode)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> openReq = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup};
    if (!openReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_open(nullptr, openReq.get(), fileInfo.path.get(), flags, mode, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open srcFile with ret: %{public}d", ret);
        return ret;
    }
    fileInfo.fdg = CreateUniquePtr<DistributedFS::FDGuard>(ret, true);
    if (fileInfo.fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(ret);
        return ENOMEM;
    }
    return ERRNO_NOERR;
}

static int32_t OpenFile(FileInfo &srcFile, FileInfo &destFile)
{
    if (srcFile.isPath) {
        auto openResult = OpenCore(srcFile, UV_FS_O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (openResult) {
            return openResult;
        }
    }
    struct stat statbf;
    if (fstat(srcFile.fdg->GetFD(), &statbf) < 0) {
        HILOGE("Failed to get stat of file by fd: %{public}d", srcFile.fdg->GetFD());
        return errno;
    }
    if (destFile.isPath) {
        auto openResult = OpenCore(destFile, UV_FS_O_RDWR | UV_FS_O_CREAT | UV_FS_O_TRUNC,
                                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (openResult) {
            return openResult;
        }
    } else {
        auto truncateResult = TruncateCore(destFile);
        if (truncateResult) {
            return truncateResult;
        }
        auto ret = lseek(destFile.fdg->GetFD(), 0, SEEK_SET);
        if (ret < 0) {
            HILOGE("Failed to lseek destFile, errno: %{public}d", errno);
            return errno;
        }
    }
    if (statbf.st_size == 0) {
        return ERRNO_NOERR;
    }
    return SendFileCore(srcFile, destFile, statbf);
}

static tuple<bool, int32_t> ValidMode(const optional<int32_t> &mode)
{
    int modeValue = 0;
    if (mode.has_value()) {
        modeValue = mode.value();
        if (modeValue) {
            return { false, modeValue };
        }
    }
    return { true, modeValue };
}

FsResult<void> CopyFileCore::DoCopyFile(FileInfo &src, FileInfo &dest,
                                        const optional<int32_t> &mode)
{
    auto [succMode, modeValue] = ValidMode(mode);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        return FsResult<void>::Error(EINVAL);
    }

    if (src.isPath && dest.isPath) {
        auto err = IsAllPath(src, dest);
        if (err) {
            return FsResult<void>::Error(err);
        }
    } else {
        auto err = OpenFile(src, dest);
        if (err) {
            return FsResult<void>::Error(err);
        }
    }
    return FsResult<void>::Success();
}

} // namespace OHOS::FileManagement::ModuleFileIO