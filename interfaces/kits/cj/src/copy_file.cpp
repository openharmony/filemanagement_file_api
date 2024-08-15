/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "stat_impl.h"
#include "macro.h"
#include "n_error.h"

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

namespace OHOS {
namespace CJSystemapi {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::FileManagement;
using namespace OHOS::CJSystemapi::FileFs;

std::tuple<int, FileInfo> ParseOperand(int32_t file)
{
    LOGI("FS_TEST:: FS_TEST::ParseOperand");
    if (file < 0) {
        LOGE("Invalid fd");
        return { EINVAL, FileInfo { false, {}, {} } };
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(file, false);
    if (fdg == nullptr) {
        LOGE("Failed to request heap memory.");
        return { ENOMEM, FileInfo { false, {}, {} } };
    }
    LOGI("FS_TEST:: FS_TEST::ParseOperand success");
    return { SUCCESS_CODE, FileInfo { false, {}, move(fdg) } };
};

std::tuple<int, FileInfo> ParseOperand(std::string file)
{
    LOGI("FS_TEST:: ParseOperand");
    std::unique_ptr<char[]> filePath = std::make_unique<char[]>(file.length() + 1);
    if (!filePath) {
        return { ENOMEM, FileInfo { true, {}, {} } };
    }
    for (size_t i = 0; i < file.length(); i++) {
        filePath[i] = file[i];
    }

    LOGI("FS_TEST:: ParseOperand success");
    return { SUCCESS_CODE, FileInfo { true, move(filePath), {} } };
};

static int IsAllPath(FileInfo& srcFile, FileInfo& destFile)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    filesystem::path srcPath(string(srcFile.path.get()));
    filesystem::path dstPath(string(destFile.path.get()));
    error_code errCode;
    LOGI("srcPath: %{public}s", srcPath.c_str());
    LOGI("dstPath: %{public}s", dstPath.c_str());
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        LOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
#else
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> copyfile_req = {
        new (nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!copyfile_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_copyfile(nullptr, copyfile_req.get(), srcFile.path.get(), destFile.path.get(),
                             UV_FS_COPYFILE_FICLONE, nullptr);
    if (ret < 0) {
        LOGE("Failed to copy file when all parameters are paths");
        return ret;
    }
#endif
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static int SendFileCore(FileInfo& srcFdg, FileInfo& destFdg, struct stat& statbf)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> sendfile_req = {
        new (nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!sendfile_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int64_t offset = 0;
    size_t size = static_cast<size_t>(statbf.st_size);
    while (size > 0) {
        int ret = uv_fs_sendfile(nullptr, sendfile_req.get(), destFdg.fdg->GetFD(), srcFdg.fdg->GetFD(),
            offset, MAX_SIZE, nullptr);
        if (ret < 0) {
            LOGE("Failed to sendfile by ret : %{public}d", ret);
            return ret;
        }
        offset += static_cast<int64_t>(ret);
        size -= static_cast<size_t>(ret);
        if (ret == 0) {
            break;
        }
    }
    if (size != 0) {
        LOGE("The execution of the sendfile task was terminated, remaining file size %{public}zu", size);
        return EIO;
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static int TruncateCore(const FileInfo& fileInfo)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> ftruncate_req = {
        new (nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!ftruncate_req) {
        LOGE("Failed to request heap memory.");
        return 1;
    }
    int ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), fileInfo.fdg->GetFD(), 0, nullptr);
    if (ret < 0) {
        LOGE("Failed to truncate dstFile with ret: %{public}d", ret);
        return 1;
    }
    return 0;
}

static int OpenCore(FileInfo& fileInfo, const int flags, const int mode)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new (nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!open_req) {
        LOGE("Failed to request heap memory.");
        return 1;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), flags, mode, nullptr);
    if (ret < 0) {
        LOGE("Failed to open srcFile with ret: %{public}d", ret);
        return 1;
    }
    fileInfo.fdg = CreateUniquePtr<DistributedFS::FDGuard>(ret, true);
    if (fileInfo.fdg == nullptr) {
        LOGE("Failed to request heap memory.");
        close(ret);
        return 1;
    }
    return 0;
}

static int OpenFile(FileInfo& srcFile, FileInfo& destFile)
{
    if (srcFile.isPath) {
        auto openResult = OpenCore(srcFile, UV_FS_O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (openResult) {
            return openResult;
        }
    }
    struct stat statbf;
    if (fstat(srcFile.fdg->GetFD(), &statbf) < 0) {
        LOGE("Failed to get stat of file by fd: %{public}d", srcFile.fdg->GetFD());
        return errno;
    }
    if (destFile.isPath) {
        auto openResult = OpenCore(destFile, UV_FS_O_RDWR | UV_FS_O_CREAT |
            UV_FS_O_TRUNC, statbf.st_mode);
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
            LOGE("Failed to lseek destFile, errno: %{public}d", errno);
            return errno;
        }
    }
    return SendFileCore(srcFile, destFile, statbf);
}

int CopyFileImpl::CopyFile(const std::string& src, const std::string& dest, int mode)
{
    LOGI("FS_TEST:: CopyFile::CopyFile start");
    auto [succSrc, srcFileInfo] = ParseOperand(src);
    if (succSrc != SUCCESS_CODE) {
        return succSrc;
    }
    auto [succDest, destFileInfo] = ParseOperand(dest);
    if (succDest != SUCCESS_CODE) {
        return succDest;
    }
    return IsAllPath(srcFileInfo, destFileInfo);
}

int CopyFileImpl::CopyFile(const std::string& src, int32_t dest, int mode)
{
    LOGI("FS_TEST:: CopyFile::CopyFile start");
    auto [succSrc, srcFileInfo] = ParseOperand(src);
    if (succSrc != SUCCESS_CODE) {
        return succSrc;
    }
    auto [succDest, destFileInfo] = ParseOperand(dest);
    if (succDest != SUCCESS_CODE) {
        return succDest;
    }
    return OpenFile(srcFileInfo, destFileInfo);
}

int CopyFileImpl::CopyFile(int32_t src, const std::string& dest, int mode)
{
    LOGI("FS_TEST:: CopyFile::CopyFile start");
    auto [succSrc, srcFileInfo] = ParseOperand(src);
    if (succSrc != SUCCESS_CODE) {
        return succSrc;
    }
    auto [succDest, destFileInfo] = ParseOperand(dest);
    if (succDest != SUCCESS_CODE) {
        return succDest;
    }
    return OpenFile(srcFileInfo, destFileInfo);
}

int CopyFileImpl::CopyFile(int32_t src, int32_t dest, int mode)
{
    LOGI("FS_TEST:: CopyFile::CopyFile start");
    auto [succSrc, srcFileInfo] = ParseOperand(src);
    if (succSrc != SUCCESS_CODE) {
        return succSrc;
    }
    auto [succDest, destFileInfo] = ParseOperand(dest);
    if (succDest != SUCCESS_CODE) {
        return succDest;
    }
    return OpenFile(srcFileInfo, destFileInfo);
}

}
}