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

#include "file_fs_impl.h"

using namespace OHOS;
using namespace OHOS::FFI;
using namespace OHOS::FileManagement;
using namespace OHOS::CJSystemapi;
using namespace OHOS::CJSystemapi::FileFs;

namespace {

std::tuple<int, FileInfo> ParseFile(int32_t file)
{
    if (file < 0) {
        LOGE("Invalid fd");
        return { EINVAL, FileInfo { false, {}, {} } };
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(file, false);
    if (fdg == nullptr) {
        LOGE("Failed to request heap memory.");
        return { ENOMEM, FileInfo { false, {}, {} } };
    }
    return { SUCCESS_CODE, FileInfo { false, {}, move(fdg) } };
};

std::tuple<int, FileInfo> ParseFile(std::string file)
{
    std::unique_ptr<char[]> filePath = std::make_unique<char[]>(file.length() + 1);
    for (size_t i = 0; i < file.length(); i++) {
        filePath[i] = file[i];
    }

    return { SUCCESS_CODE, FileInfo { true, move(filePath), {} } };
};

int CheckFsStat(const FileInfo &fileInfo, uv_fs_t* req)
{
    if (fileInfo.isPath) {
        int ret = uv_fs_stat(nullptr, req, fileInfo.path.get(), nullptr);
        if (ret < 0) {
            LOGE("Failed to stat file with path");
            return ret;
        }
    } else {
        int ret = uv_fs_fstat(nullptr, req, fileInfo.fdg->GetFD(), nullptr);
        if (ret < 0) {
            LOGE("Failed to stat file with fd");
            return ret;
        }
    }
    return SUCCESS_CODE;
}

std::tuple<int, uv_stat_t*> GetUvStat(const FileInfo& fileInfo)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        LOGE("Failed to request heap memory.");
        return {ENOMEM, nullptr};
    }
    auto state = CheckFsStat(fileInfo, stat_req.get());
    if (state != SUCCESS_CODE) {
        return { state, nullptr };
    }
    return {SUCCESS_CODE, &stat_req->statbuf};
}

std::tuple<bool, FileInfo, int> ParseRandomFile(std::string file)
{
    LOGI("FS_TEST:: RandomAccessFileImpl::ParseRandomFile");
    std::unique_ptr<char[]> filePath = std::make_unique<char[]>(file.length() + 1);
    for (size_t i = 0; i < file.length(); i++) {
        filePath[i] = file[i];
    }
    OHOS::DistributedFS::FDGuard sfd;
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(sfd, false);
    if (fdg == nullptr) {
        return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
    }
    LOGI("FS_TEST:: RandomAccessFileImpl::ParseRandomFile success");
    return { true, FileInfo { true, move(filePath), move(fdg) }, ERRNO_NOERR};
}

std::tuple<int, bool> GetFsAccess(const FileInfo &fileInfo)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        LOGE("Failed to request heap memory.");
        return {ENOMEM, false};
    }
    bool isAccess = false;
    int ret = uv_fs_access(nullptr, stat_req.get(), fileInfo.path.get(), 0, nullptr);
    if (ret < 0 && (std::string_view(uv_err_name(ret)) != "ENOENT")) {
        LOGE("Failed to access file by path");
        return {ret, false};
    }
    if (ret == 0) {
        isAccess = true;
    }
    return {SUCCESS_CODE, isAccess};
}
}

namespace OHOS {
namespace CJSystemapi {
using namespace std;
std::tuple<int, sptr<StatImpl>> FileFsImpl::Stat(int32_t file)
{
    auto [fileState, fileInfo] = ParseFile(file);

    if (fileState != SUCCESS_CODE) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
    auto [statState, stat] = GetUvStat(fileInfo);
    if (statState != SUCCESS_CODE) {
        return {GetErrorCode(statState), nullptr};
    }
    auto nativeStat = FFIData::Create<StatImpl>(*stat);
    return {SUCCESS_CODE, nativeStat};
}

std::tuple<int32_t, sptr<StatImpl>> FileFsImpl::Stat(std::string file)
{
    auto [fileState, fileInfo] = ParseFile(file);

    if (fileState != SUCCESS_CODE) {
        LOGE("ParseFile false, please check the file path!");
        return {GetErrorCode(fileState), nullptr};
    }
    auto [statState, stat] = GetUvStat(fileInfo);
    if (statState != SUCCESS_CODE) {
        LOGE("Failed to getUvStat file by fileInfo");
        return {GetErrorCode(statState), nullptr};
    }
    auto nativeStat = FFIData::Create<StatImpl>(*stat);
    return {SUCCESS_CODE, nativeStat};
}

std::tuple<int32_t, sptr<StreamImpl>> FileFsImpl::CreateStream(std::string path, std::string mode)
{
    std::unique_ptr<FILE, decltype(&fclose)> fp = { fopen(path.c_str(), mode.c_str()), fclose };
    if (!fp) {
        LOGE("Failed to fdopen file by path");
        return {GetErrorCode(errno), nullptr};
    }
    auto nativeStream = FFIData::Create<StreamImpl>(std::move(fp));
    return {SUCCESS_CODE, nativeStream};
}

std::tuple<int32_t, sptr<StreamImpl>> FileFsImpl::FdopenStream(int32_t fd, std::string mode)
{
    LOGI("FS_TEST::FileFsImpl::FdopenStream start");
    if (fd < 0) {
        LOGE("Invalid fd");
        return {GetErrorCode(EINVAL), nullptr};
    }
    unique_ptr<FILE, decltype(&fclose)> fp = { fdopen(fd, mode.c_str()), fclose };
    if (!fp) {
        LOGE("Failed to fdopen file by fd:%{public}d", fd);
        return {GetErrorCode(errno), nullptr};
    }
    auto nativeStream = FFIData::Create<StreamImpl>(std::move(fp));
    return {SUCCESS_CODE, nativeStream};
}

std::tuple<int32_t, sptr<StatImpl>> FileFsImpl::Lstat(std::string path)
{
    LOGI("FS_TEST::FileFsImpl::Lstat start");

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> lstat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!lstat_req) {
        LOGE("Failed to request heap memory.");
        return {GetErrorCode(ENOMEM), nullptr};
    }
    int ret = uv_fs_lstat(nullptr, lstat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to get stat of file, ret: %{public}d", ret);
        return {GetErrorCode(ret), nullptr};
    }
    auto nativeStat = FFIData::Create<StatImpl>(lstat_req->statbuf);
    return {SUCCESS_CODE, nativeStat};
}

std::tuple<int32_t, sptr<RandomAccessFileImpl>> FileFsImpl::CreateRandomAccessFileSync(std::string file,
    unsigned int mode)
{
    auto [succ, fileInfo, err] = ParseRandomFile(file);
    if (!succ) {
        LOGE("Error because %{public}d", err);
        return {GetErrorCode(err), nullptr};
    }
 
    if (fileInfo.isPath) {
        if (!succ || mode < 0) {
            LOGE("Invalid flags");
            return {GetErrorCode(EINVAL), nullptr};
        }
        unsigned int flags = static_cast<unsigned int>(mode);
        CommonFunc::ConvertCjFlags(flags);
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
        if (!open_req) {
            LOGE("Failed to request heap memory.");
            return {GetErrorCode(ENOMEM), nullptr};
        }
 
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), flags, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, NULL);
        if (ret < 0) {
            LOGE("Faile in fs_open %{public}d", ret);
            return {GetErrorCode(ret), nullptr};
        }
        fileInfo.fdg->SetFD(open_req.get()->result, false);
    }
    std::shared_ptr<OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity> ptr =
            std::make_shared<OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity>();
    ptr->fd.swap(fileInfo.fdg);
    ptr->filePointer = 0;
    auto randomAccessFileImpl = FFIData::Create<RandomAccessFileImpl>(std::move(ptr));
    return {SUCCESS_CODE, randomAccessFileImpl};
}

std::tuple<int32_t, sptr<RandomAccessFileImpl>> FileFsImpl::CreateRandomAccessFileSync(sptr<FileEntity> entity,
    unsigned int mode)
{
    auto fd = entity->fd_.get()->GetFD();
    FileInfo fileInfo;
    if (fd < 0) {
        HILOGE("Invalid fd");
        return { GetErrorCode(EINVAL), nullptr };
    }
    auto dupFd = dup(fd);
    if (dupFd < 0) {
        HILOGE("Failed to get valid fd, fail reason: %{public}s, fd: %{public}d", strerror(errno), fd);
        return { GetErrorCode(EINVAL), nullptr};
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(dupFd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        return { GetErrorCode(ENOMEM), nullptr};
    }
    fileInfo = FileInfo { false, nullptr, move(fdg) };
    std::shared_ptr<OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity> ptr =
        std::make_shared<OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity>();
    ptr->fd.swap(fileInfo.fdg);
    ptr->filePointer = 0;
    auto randomAccessFileImpl = FFIData::Create<RandomAccessFileImpl>(std::move(ptr));
    return {SUCCESS_CODE, randomAccessFileImpl};
}

int FileFsImpl::Mkdir(std::string path, bool recursion, bool isTwoArgs)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (isTwoArgs) {
        MakeDirectionMode recursionMode = SINGLE;
        recursionMode = static_cast<MakeDirectionMode>(recursion);
        if (::Mkdirs(path.c_str(), recursionMode) < 0) {
            HILOGE("Failed to create directories, error: %{public}d", errno);
            return GetErrorCode(errno);
        }
        return SUCCESS_CODE;
    }
#endif
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> mkdir_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!mkdir_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    int ret = uv_fs_mkdir(nullptr, mkdir_req.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
    if (ret) {
        HILOGE("Failed to create directory");
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}
 
int FileFsImpl::Rmdir(std::string path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> scandir_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    int ret = 0;
    ret = uv_fs_scandir(nullptr, scandir_req.get(), path.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to scandir, ret: %{public}d", ret);
        return GetErrorCode(ret);
    }
    uv_dirent_t dent;
    while (uv_fs_scandir_next(scandir_req.get(), &dent) != UV_EOF) {
        string filePath = path + "/" + string(dent.name);
        if (dent.type == UV_DIRENT_FILE) {
            std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> unlink_req = {
                new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
            if (!unlink_req) {
                HILOGE("Failed to request heap memory.");
                return GetErrorCode(ENOMEM);
            }
            ret = uv_fs_unlink(nullptr, unlink_req.get(), filePath.c_str(), nullptr);
            if (ret < 0) {
                HILOGE("Failed to unlink file, ret: %{public}d", ret);
                return GetErrorCode(ret);
            }
        } else if (dent.type == UV_DIRENT_DIR) {
            auto rmDirentRes = Rmdir(filePath);
            if (rmDirentRes) {
                return rmDirentRes;
            }
        }
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> rmdir_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!rmdir_req) {
        LOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    ret = uv_fs_rmdir(nullptr, rmdir_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to rmdir empty dir, ret: %{public}d", ret);
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}
 
int FileFsImpl::Rename(std::string oldPath, std::string newPath)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> rename_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!rename_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    int ret = uv_fs_rename(nullptr, rename_req.get(), oldPath.c_str(), newPath.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to rename file with path");
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}
 
int FileFsImpl::Unlink(std::string path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> unlink_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!unlink_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    int ret = uv_fs_unlink(nullptr, unlink_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to unlink with path");
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ConflictFiles> &errfiles);

static tuple<bool, bool> JudgeExistAndEmpty(const string &path)
{
    filesystem::path pathName(path);
    if (filesystem::exists(pathName)) {
        if (filesystem::is_empty(pathName)) {
            return { true, true };
        }
        return { true, false };
    }
    return { false, false };
}

static int RmDirectory(const string &path)
{
    filesystem::path pathName(path);
    if (filesystem::exists(pathName)) {
        std::error_code errCode;
        (void)filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            LOGE("Failed to remove directory, error code: %{public}d", errCode.value());
            return errCode.value();
        }
    }
    return ERRNO_NOERR;
}

static int RestoreTime(const string &srcPath, const string &destPath)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_stat(nullptr, stat_req.get(), srcPath.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to stat srcPath");
        return ret;
    }
    double atime = static_cast<double>(stat_req->statbuf.st_atim.tv_sec) +
        static_cast<double>(stat_req->statbuf.st_atim.tv_nsec) / NS;
    double mtime = static_cast<double>(stat_req->statbuf.st_mtim.tv_sec) +
        static_cast<double>(stat_req->statbuf.st_mtim.tv_nsec) / NS;
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> utime_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!utime_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    ret = uv_fs_utime(nullptr, utime_req.get(), destPath.c_str(), atime, mtime, nullptr);
    if (ret < 0) {
        HILOGE("Failed to utime %s, error code: %d", destPath.c_str(), ret);
        return ret;
    }
    return ERRNO_NOERR;
}

struct NameListArg {
    struct dirent** namelist;
    int num;
};

static void Deleter(struct NameListArg *arg)
{
    for (int i = 0; i < arg->num; i++) {
        delete (arg->namelist)[i];
        (arg->namelist)[i] = nullptr;
    }
    delete arg->namelist;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILE_DISMATCH;
    }
    return FILE_MATCH;
}

static int RemovePath(const string& pathStr)
{
    filesystem::path pathTarget(pathStr);
    std::error_code errCode;
    if (!filesystem::remove(pathTarget, errCode)) {
        HILOGE("Failed to remove file or directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}


static int RenameDir(const string &src, const string &dest, const int mode, deque<struct ConflictFiles> &errfiles)
{
    filesystem::path destPath(dest);
    if (filesystem::exists(destPath)) {
        return RecurMoveDir(src, dest, mode, errfiles);
    }
    filesystem::path srcPath(src);
    std::error_code errCode;
    filesystem::rename(srcPath, destPath, errCode);
    if (errCode.value() == EXDEV) {
        HILOGE("Failed to rename file due to EXDEV");
        if (!filesystem::create_directory(destPath, errCode)) {
            HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
            return errCode.value();
        }
        int ret = RestoreTime(srcPath, destPath);
        if (ret) {
            HILOGE("Failed to utime dstPath");
            return ret;
        }
        return RecurMoveDir(src, dest, mode, errfiles);
    }
    if (errCode.value() != 0) {
        HILOGE("Failed to rename file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

static int CopyAndDeleteFile(const string &src, const string &dest)
{
    filesystem::path dstPath(dest);
    if (filesystem::exists(dstPath)) {
        int removeRes = RemovePath(dest);
        if (removeRes != 0) {
            HILOGE("Failed to remove dest file");
            return removeRes;
        }
    }
    filesystem::path srcPath(src);
    std::error_code errCode;
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    int ret = RestoreTime(srcPath, dstPath);
    if (ret) {
        HILOGE("Failed to utime dstPath");
        return ret;
    }
    return RemovePath(src);
}
