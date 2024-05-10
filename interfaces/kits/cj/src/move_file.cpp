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

#include "move_file.h"
#include "macro.h"
#include "stat_impl.h"

#include <cstring>
#include <fcntl.h>

#ifdef __MUSL__
#include <filesystem>
#else
#include <sys/stat.h>
#endif

#include "uv.h"
#include <tuple>
#include <unistd.h>

namespace OHOS {
namespace CJSystemapi {
using namespace std;
using namespace OHOS::FileManagement::LibN;

#ifdef __MUSL__
static bool CheckDir(const string &path)
{
    if (!filesystem::is_directory(filesystem::status(path))) {
        return false;
    }
    return true;
}
#else
static bool CheckDir(const string &path)
{
    struct stat fileInformation;
    if (stat(path.c_str(), &fileInformation) == 0) {
        if (fileInformation.st_mode & S_IFDIR) {
            return true;
        }
    } else {
        LOGE("Failed to stat file");
    }
    return false;
}
#endif

static int ChangeTime(const string &path, uv_fs_t *statReq)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> utime_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!utime_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    double atime = static_cast<double>(statReq->statbuf.st_atim.tv_sec) +
        static_cast<double>(statReq->statbuf.st_atim.tv_nsec) / NS;
    double mtime = static_cast<double>(statReq->statbuf.st_mtim.tv_sec) +
        static_cast<double>(statReq->statbuf.st_mtim.tv_nsec) / NS;
    int ret = uv_fs_utime(nullptr, utime_req.get(), path.c_str(), atime, mtime, nullptr);
    if (ret < 0) {
        LOGE("Failed to utime dstPath");
    }
    return ret;
}
static int CopyAndDeleteFile(const string &src, const string &dest)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_stat(nullptr, stat_req.get(), src.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to stat srcPath");
        return ret;
    }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    filesystem::path dstPath(dest);
    std::error_code errCode;
    if (filesystem::exists(dstPath)) {
        if (!filesystem::remove(dstPath, errCode)) {
            LOGE("Failed to remove dest file, error code: %{public}d", errCode.value());
            return errCode.value();
        }
    }
    filesystem::path srcPath(src);
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        LOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
#else
    uv_fs_t copyfile_req;
    ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), MODE_FORCE_MOVE, nullptr);
    uv_fs_req_cleanup(&copyfile_req);
    if (ret < 0) {
        LOGE("Failed to move file using copyfile interface.");
        return ret;
    }
#endif
    uv_fs_t unlinkReq;
    ret = uv_fs_unlink(nullptr, &unlinkReq, src.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to unlink src file");
        int result = uv_fs_unlink(nullptr, &unlinkReq, dest.c_str(), nullptr);
        if (result < 0) {
            LOGE("Failed to unlink dest file");
            return result;
        }
        uv_fs_req_cleanup(&unlinkReq);
        return ret;
    }
    uv_fs_req_cleanup(&unlinkReq);
    return ChangeTime(dest, stat_req.get());
}

static int RenameFile(const string &src, const string &dest)
{
    int ret = 0;
    uv_fs_t renameReq;
    ret = uv_fs_rename(nullptr, &renameReq, src.c_str(), dest.c_str(), nullptr);
    if (ret < 0 && (string_view(uv_err_name(ret)) == "EXDEV")) {
        return CopyAndDeleteFile(src, dest);
    }
    if (ret < 0) {
        LOGE("Failed to move file using rename syscall.");
        return ret;
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

int MoveFileImpl::MoveFile(std::string src, std::string dest, int mode)
{
    LOGI("FS_TEST:: MoveFileImpl::MoveFile start");
    if (CheckDir(src)) {
        LOGE("Invalid src");
        return EINVAL;
    }
    if (CheckDir(dest)) {
        LOGE("Invalid dest");
        return EINVAL;
    }
    uv_fs_t accessReq;
    int ret = uv_fs_access(nullptr, &accessReq, src.c_str(), W_OK, nullptr);
    if (ret < 0) {
        LOGE("Failed to move src file due to doesn't exist or hasn't write permission");
        uv_fs_req_cleanup(&accessReq);
        return ret;
    }
    if (mode == MODE_THROW_ERR) {
        ret = uv_fs_access(nullptr, &accessReq, dest.c_str(), 0, nullptr);
        uv_fs_req_cleanup(&accessReq);
        if (ret == 0) {
            LOGE("Failed to move file due to existing destPath with MODE_THROW_ERR.");
            return EEXIST;
        }
        if (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) {
            LOGE("Failed to access destPath with MODE_THROW_ERR.");
            return ret;
        }
    } else {
        uv_fs_req_cleanup(&accessReq);
    }
    return RenameFile(src, dest);
}

}
}