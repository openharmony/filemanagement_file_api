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
    if (!filePath) {
        return { ENOMEM, FileInfo { true, nullptr, {} } };
    }
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
    uv_stat_t* tempBuf = new (std::nothrow) uv_stat_t(stat_req->statbuf);
    if (!tempBuf) {
        return {ENOMEM, nullptr};
    }
    return {SUCCESS_CODE, tempBuf};
}

std::tuple<bool, FileInfo, int> ParseRandomFile(std::string file)
{
    LOGI("FS_TEST:: RandomAccessFileImpl::ParseRandomFile");
    std::unique_ptr<char[]> filePath = std::make_unique<char[]>(file.length() + 1);
    if (!filePath) {
        return { false, FileInfo { true, nullptr, {} }, ENOMEM };
    }
    for (size_t i = 0; i < file.length(); i++) {
        filePath[i] = file[i];
    }
    OHOS::DistributedFS::FDGuard sfd;
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(sfd, false);
    if (fdg == nullptr) {
        return { false, FileInfo { false, nullptr, nullptr }, ENOMEM };
    }
    LOGI("FS_TEST:: RandomAccessFileImpl::ParseRandomFile success");
    return { true, FileInfo { true, move(filePath), move(fdg) }, ERRNO_NOERR };
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
        return {GetErrorCode(fileState), nullptr};
    }
    auto [statState, stat] = GetUvStat(fileInfo);
    if (statState != SUCCESS_CODE) {
        return {GetErrorCode(statState), nullptr};
    }
    auto nativeStat = FFIData::Create<StatImpl>(*stat);
    delete(stat);
    stat = nullptr;
    if (!nativeStat) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
    delete(stat);
    stat = nullptr;
    if (!nativeStat) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
    if (!nativeStream) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
    if (!nativeStream) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
    if (!nativeStat) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
        if (mode < 0) {
            LOGE("Invalid flags");
            return {GetErrorCode(EINVAL), nullptr};
        }
        unsigned int flags = static_cast<unsigned int>(mode);
        CommonFunc::ConvertCjFlags(flags);
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
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
    if (!randomAccessFileImpl) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
    if (!randomAccessFileImpl) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
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
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
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
    if (!scandir_req) {
        return GetErrorCode(ENOMEM);
    }
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
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
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
    arg->namelist = nullptr;
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

static int RenameFile(const string &src, const string &dest, const int mode, deque<struct ConflictFiles> &errfiles)
{
    filesystem::path dstPath(dest);
    if (filesystem::exists(dstPath)) {
        if (filesystem::is_directory(dstPath)) {
            errfiles.emplace_front(src, dest);
            return ERRNO_NOERR;
        }
        if (mode == DIRMODE_FILE_THROW_ERR) {
            errfiles.emplace_back(src, dest);
            return ERRNO_NOERR;
        }
    }
    filesystem::path srcPath(src);
    std::error_code errCode;
    filesystem::rename(srcPath, dstPath, errCode);
    if (errCode.value() == EXDEV) {
        HILOGE("Failed to rename file due to EXDEV");
        return CopyAndDeleteFile(src, dest);
    }
    return errCode.value();
}

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ConflictFiles> &errfiles)
{
    filesystem::path dpath(destPath);
    if (!filesystem::is_directory(dpath)) {
        errfiles.emplace_front(srcPath, destPath);
        return ERRNO_NOERR;
    }

    unique_ptr<struct NameListArg, decltype(Deleter)*> ptr = {new struct NameListArg, Deleter};
    if (!ptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(ptr->namelist), FilterFunc, alphasort);
    ptr->num = num;

    for (int i = 0; i < num; i++) {
        if ((ptr->namelist[i])->d_type == DT_DIR) {
            string srcTemp = srcPath + '/' + string((ptr->namelist[i])->d_name);
            string destTemp = destPath + '/' + string((ptr->namelist[i])->d_name);
            size_t size = errfiles.size();
            int res = RenameDir(srcTemp, destTemp, mode, errfiles);
            if (res != ERRNO_NOERR) {
                return res;
            }
            if (size != errfiles.size()) {
                continue;
            }
            res = RemovePath(srcTemp);
            if (res) {
                return res;
            }
        } else {
            string src = srcPath + '/' + string((ptr->namelist[i])->d_name);
            string dest = destPath + '/' + string((ptr->namelist[i])->d_name);
            int res = RenameFile(src, dest, mode, errfiles);
            if (res != ERRNO_NOERR) {
                HILOGE("Failed to rename file for error %{public}d", res);
                return res;
            }
        }
    }
    return ERRNO_NOERR;
}

static int MoveDirFunc(const string &src, const string &dest, const int mode,
    std::deque<struct ConflictFiles> &errfiles)
{
    size_t found = string(src).rfind('/');
    if (found == std::string::npos) {
        return EINVAL;
    }
    if (access(src.c_str(), W_OK) != 0) {
        LOGE("Failed to move src directory due to doesn't exist or hasn't write permission");
        return errno;
    }
    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    auto [destStrExist, destStrEmpty] = JudgeExistAndEmpty(destStr);
    if (destStrExist && !destStrEmpty) {
        if (mode == DIRMODE_DIRECTORY_REPLACE) {
            int removeRes = RmDirectory(destStr);
            if (removeRes) {
                HILOGE("Failed to remove dest directory in DIRMODE_DIRECTORY_REPLACE");
                return removeRes;
            }
        }
        if (mode == DIRMODE_DIRECTORY_THROW_ERR) {
            HILOGE("Failed to move directory in DIRMODE_DIRECTORY_THROW_ERR");
            return ENOTEMPTY;
        }
    }
    int res = RenameDir(src, destStr, mode, errfiles);
    if (res == ERRNO_NOERR) {
        if (!errfiles.empty()) {
            HILOGE("Failed to movedir with some conflicted files");
            return EEXIST;
        }
        int removeRes = RmDirectory(src);
        if (removeRes) {
            HILOGE("Failed to remove src directory");
            return removeRes;
        }
    }
    return res;
}

static CConflictFiles* DequeToCConflict(std::deque<struct ConflictFiles> errfiles)
{
    CConflictFiles* result = new(std::nothrow) CConflictFiles[errfiles.size()];
    if (result == nullptr) {
        return nullptr;
    }
    size_t temp = 0;
    for (size_t i = 0; i < errfiles.size(); i++) {
        size_t srcFilesLen = errfiles[i].srcFiles.length() + 1;
        result[i].srcFiles = static_cast<char*>(malloc(srcFilesLen));
        if (result[i].srcFiles == nullptr) {
            break;
        }
        if (strcpy_s(result[i].srcFiles, srcFilesLen, errfiles[i].srcFiles.c_str()) != 0) {
            free(result[i].srcFiles);
            result[i].srcFiles = nullptr;
            break;
        }
        size_t destFilesLen = errfiles[i].destFiles.length() + 1;
        result[i].destFiles = static_cast<char*>(malloc(destFilesLen));
        if (result[i].destFiles == nullptr) {
            free(result[i].srcFiles);
            result[i].srcFiles = nullptr;
            break;
        }
        if (strcpy_s(result[i].destFiles, destFilesLen, errfiles[i].destFiles.c_str()) != 0) {
            free(result[i].srcFiles);
            free(result[i].destFiles);

            result[i].srcFiles = nullptr;
            result[i].destFiles = nullptr;
            break;
        }
        temp++;
    }
    if (temp != errfiles.size()) {
        for (size_t j = temp; j > 0; j--) {
            free(result[j - 1].srcFiles);
            free(result[j - 1].destFiles);

            result[j - 1].srcFiles = nullptr;
            result[j - 1].destFiles = nullptr;
        }
        delete[] result;
        result = nullptr;
        return nullptr;
    }
    return result;
}

RetDataCArrConflictFiles FileFsImpl::MoveDir(string src, string dest, int32_t mode)
{
    RetDataCArrConflictFiles ret = { .code = EINVAL, .data = { .head = nullptr, .size = 0 } };
    if (!filesystem::is_directory(filesystem::status(src))) {
        HILOGE("Invalid src");
        ret.code = GetErrorCode(EINVAL);
        return ret;
    }
    if (!filesystem::is_directory(filesystem::status(dest))) {
        HILOGE("Invalid dest");
        ret.code = GetErrorCode(EINVAL);
        return ret;
    }
    if (mode < DIRMODE_MIN || mode > DIRMODE_MAX) {
        HILOGE("Invalid mode");
        ret.code = GetErrorCode(EINVAL);
        return ret;
    }
    std::deque<struct ConflictFiles> errfiles = {};
    int code = MoveDirFunc(src, dest, mode, errfiles);
    if (code != SUCCESS_CODE) {
        ret.code = GetErrorCode(code);
    } else {
        ret.code = SUCCESS_CODE;
    }
    ret.data.size = (int64_t)errfiles.size();
    ret.data.head = DequeToCConflict(errfiles);
    return ret;
}

static bool CheckReadArgs(int32_t fd, const char* buf, int64_t bufLen, size_t length, int64_t offset)
{
    if (fd < 0) {
        LOGE("Invalid fd");
        return false;
    }
    if (buf == nullptr) {
        LOGE("malloc fail");
        return false;
    }
    if (bufLen > UINT_MAX) {
        LOGE("Invalid arraybuffer");
        return false;
    }
    if (length > UINT_MAX) {
        LOGE("Invalid arraybuffer");
        return false;
    }
    if (offset < 0) {
        LOGE("option.offset shall be positive number");
        return false;
    }
    return true;
}

RetDataI64 FileFsImpl::Read(int32_t fd, char* buf, int64_t bufLen, size_t length, int64_t offset)
{
    LOGI("FS_TEST::FileFsImpl::Read start");
    RetDataI64 ret = { .code = EINVAL, .data = 0 };

    if (!CheckReadArgs(fd, buf, bufLen, length, offset)) {
        return ret;
    }

    auto [state, buff, len, offsetResult] = GetReadArg(static_cast<size_t>(bufLen), length, offset);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> read_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!read_req) {
        LOGE("Failed to request heap memory.");
        ret.code = ENOMEM;
        return ret;
    }
    int readCode = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, offset, nullptr);
    if (readCode < 0) {
        LOGE("Failed to read file for %{public}d", readCode);
        ret.code = readCode;
        return ret;
    }
    ret.code = SUCCESS_CODE;
    ret.data = static_cast<int64_t>(readCode);
    return ret;
}

RetDataI64 FileFsImpl::ReadCur(int32_t fd, char* buf, int64_t bufLen, size_t length)
{
    LOGI("FS_TEST::FileFsImpl::Read start");
    RetDataI64 ret = { .code = EINVAL, .data = 0 };
    
    if (!CheckReadArgs(fd, buf, bufLen, length, 0)) {
        return ret;
    }

    auto [state, buff, len, offsetResult] = GetReadArg(static_cast<size_t>(bufLen), length, 0);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> read_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!read_req) {
        LOGE("Failed to request heap memory.");
        ret.code = ENOMEM;
        return ret;
    }
    int readCode = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, -1, nullptr);
    if (readCode < 0) {
        LOGE("Failed to read file for %{public}d", readCode);
        ret.code = readCode;
        return ret;
    }

    ret.code = SUCCESS_CODE;
    ret.data = static_cast<int64_t>(readCode);
    return ret;
}

RetDataI64 FileFsImpl::Write(int32_t fd, char* buf, size_t length, int64_t offset, std::string encode)
{
    LOGI("FS_TEST::FileFsImpl::Write start");
    RetDataI64 ret = { .code = EINVAL, .data = 0 };
    if (fd < 0) {
        LOGE("Invalid fd");
        return ret;
    } else if (length > UINT_MAX) {
        LOGE("Invalid arraybuffer");
        return ret;
    }
    if (offset < 0) {
        LOGE("option.offset shall be positive number");
        return ret;
    }

    auto [state, bufGuard, buff, len, offsetResult] =
        FileFs::GetWriteArg(buf, length, offset, encode);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {state, 0};
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buff), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> write_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!write_req) {
        LOGE("Failed to request heap memory.");
        ret.code = ENOMEM;
        return ret;
    }
    int writeCode = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (writeCode < 0) {
        LOGE("Failed to write file for %{public}d", writeCode);
        ret.code = writeCode;
        return ret;
    }
    ret.code = SUCCESS_CODE;
    ret.data = static_cast<int64_t>(writeCode);
    return ret;
}

RetDataI64 FileFsImpl::WriteCur(int32_t fd, char* buf, size_t length, std::string encode)
{
    LOGI("FS_TEST::FileFsImpl::Write start");
    RetDataI64 ret = { .code = EINVAL, .data = 0 };
    if (fd < 0) {
        LOGE("Invalid fd");
        return ret;
    }
    if (length > UINT_MAX) {
        LOGE("Invalid arraybuffer");
        return ret;
    }

    auto [state, bufGuard, buff, len, offsetResult] =
        FileFs::GetWriteArg(buf, length, 0, encode);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {state, 0};
    }
    
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buff), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> write_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!write_req) {
        LOGE("Failed to request heap memory.");
        ret.code = ENOMEM;
        return ret;
    }
    int writeCode = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, -1, nullptr);
    if (writeCode < 0) {
        LOGE("Failed to write file for %{public}d", writeCode);
        ret.code = writeCode;
        return ret;
    }
    ret.code = SUCCESS_CODE;
    ret.data = static_cast<int64_t>(writeCode);
    return ret;
}

std::tuple<int32_t, bool> FileFsImpl::Access(std::string path)
{
    auto [fileState, fileInfo] = ParseFile(path);
 
    if (fileState != SUCCESS_CODE) {
        return {GetErrorCode(ENOMEM), false};
    }
    auto [accessState, access] = GetFsAccess(fileInfo);
    if (accessState < 0) {
        return {GetErrorCode(accessState), false};
    }
    return {SUCCESS_CODE, access};
}

int FileFsImpl::Truncate(std::string file, int64_t len)
{
    auto [fileState, fileInfo] = ParseFile(file);
    if (fileState != SUCCESS_CODE) {
        return GetErrorCode(EINVAL);
    }
    if (len < 0) {
        return GetErrorCode(EINVAL);
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        return GetErrorCode(ret);
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> ftruncate_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!ftruncate_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), ret, len, nullptr);
    if (ret < 0) {
        HILOGE("Failed to truncate file by path");
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}

int FileFsImpl::Truncate(int32_t fd, int64_t len)
{
    auto [fileState, fileInfo] = ParseFile(fd);
    if (fileState != SUCCESS_CODE) {
        return GetErrorCode(fileState);
    }
    if (len < 0) {
        return GetErrorCode(EINVAL);
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> ftruncate_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!ftruncate_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }
    int ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), fileInfo.fdg->GetFD(), len, nullptr);
    if (ret < 0) {
        HILOGE("Failed to truncate file by fd for libuv error %{public}d", ret);
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}

static int CloseFd(int fd)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> close_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!close_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_close(nullptr, close_req.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to close file with ret: %{public}d", ret);
        return ret;
    }
    return SUCCESS_CODE;
}

static int CloseCore(FileStruct fileStruct)
{
    if (fileStruct.isFd) {
        auto err = CloseFd(fileStruct.fd);
        if (err) {
            return GetErrorCode(err);
        }
    } else {
        auto err = CloseFd(fileStruct.fileEntity->fd_->GetFD());
        if (err) {
            return GetErrorCode(err);
        }
    }
    return SUCCESS_CODE;
}

int FileFsImpl::Close(int32_t file)
{
    FileStruct fileStruct;
    if (file >= 0) {
        fileStruct = FileStruct { true, file, nullptr };
    } else {
        return GetErrorCode(EINVAL);
    }
    
    return CloseCore(fileStruct);
}

int FileFsImpl::Close(sptr<OHOS::CJSystemapi::FileFs::FileEntity> file)
{
    FileStruct fileStruct = FileStruct { false, -1, file };
    return CloseCore(fileStruct);
}

static int GetFileSize(const string &path, int64_t &offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_stat(nullptr, stat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get file stat by path");
        return ret;
    }

    offset = static_cast<int64_t>(stat_req->statbuf.st_size);
    return ERRNO_NOERR;
}

std::tuple<int32_t, sptr<ReadIteratorImpl>> FileFsImpl::ReadLines(char* file, std::string encoding)
{
    if (encoding != "utf-8") {
        return { GetErrorCode(EINVAL), nullptr};
    }
    auto iterator = ::ReaderIterator(file);
    if (iterator == nullptr) {
        HILOGE("Failed to read lines of the file, error: %{public}d", errno);
        return { GetErrorCode(errno), nullptr};
    }
    int64_t offset = 0;
    int ret = GetFileSize(file, offset);
    if (ret != 0) {
        HILOGE("Failed to get size of the file");
        return { GetErrorCode(ret), nullptr};
    }
    std::shared_ptr<OHOS::FileManagement::ModuleFileIO::ReaderIteratorEntity> ptr =
        std::make_shared<OHOS::FileManagement::ModuleFileIO::ReaderIteratorEntity>();
    ptr->iterator = iterator;
    ptr->offset = offset;
    auto readIteratorImpl = FFIData::Create<ReadIteratorImpl>(std::move(ptr));
    if (!readIteratorImpl) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
    return {SUCCESS_CODE, readIteratorImpl};
}

static int ReadTextCheckArgs(int64_t offset, int64_t len, char* encoding)
{
    if (offset < 0) {
        HILOGE("Illegal option.offset parameter");
        return GetErrorCode(EINVAL);
    }
    if (len < 0 || len > UINT_MAX) {
        HILOGE("Illegal option.length parameter");
        return GetErrorCode(EINVAL);
    }
    if (string(encoding) != "utf-8") {
        HILOGE("Illegal option.encoding parameter");
        return GetErrorCode(EINVAL);
    }
    return SUCCESS_CODE;
}

static int ReadTextOpenFile(const std::string& path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup
    };
    if (open_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }

    return uv_fs_open(nullptr, open_req.get(), path.c_str(), O_RDONLY,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
}

static int ReadFromFile(int fd, int64_t offset, string& buffer)
{
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), static_cast<unsigned int>(buffer.size()));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> read_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (read_req == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    return uv_fs_read(nullptr, read_req.get(), fd, &readbuf, 1, offset, nullptr);
}

RetDataCString FileFsImpl::ReadText(char* path, int64_t offset, bool hasLen, int64_t len, char* encoding)
{
    RetDataCString retData = { .code = ERR_INVALID_INSTANCE_CODE, .data = nullptr };
    int code = ReadTextCheckArgs(offset, len, encoding);
    if (code != SUCCESS_CODE) {
        retData.code = code;
        return retData;
    }

    DistributedFS::FDGuard sfd;
    int fd = ReadTextOpenFile(path);
    if (fd < 0) {
        HILOGE("Failed to open file by ret: %{public}d", fd);
        retData.code =  GetErrorCode(errno);
        return retData;
    }
    sfd.SetFD(fd);
    struct stat statbf;
    if ((!sfd) || (fstat(sfd.GetFD(), &statbf) < 0)) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        retData.code =  GetErrorCode(errno);
        return retData;
    }

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset: %{public}" PRIu64, offset);
        retData.code =  GetErrorCode(EINVAL);
        return retData;
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    int readRet = ReadFromFile(sfd.GetFD(), offset, buffer);
    if (readRet < 0) {
        HILOGE("Failed to read file by fd: %{public}d", sfd.GetFD());
        retData.code =  GetErrorCode(errno);
        return retData;
    }
    char *value = static_cast<char*>(malloc((len + 1) * sizeof(char)));
    if (value == nullptr) {
        return retData;
    }
    std::char_traits<char>::copy(value, buffer.c_str(), len + 1);
    retData.code = SUCCESS_CODE;
    retData.data = value;
    return retData;
}

int FileFsImpl::Utimes(std::string path, double mtime)
{
    if (mtime < 0) {
        HILOGE("Invalid mtime from JS second argument");
        return GetErrorCode(EINVAL);
    }

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }

    int ret = uv_fs_stat(nullptr, stat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of the file by path");
        return GetErrorCode(ret);
    }

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> utimes_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!utimes_req) {
        HILOGE("Failed to request heap memory.");
        return GetErrorCode(ENOMEM);
    }

    double atime = static_cast<double>(stat_req->statbuf.st_atim.tv_sec) +
        static_cast<double>(stat_req->statbuf.st_atim.tv_nsec) / NS;
    ret = uv_fs_utime(nullptr, utimes_req.get(), path.c_str(), atime, mtime / MS, nullptr);
    if (ret < 0) {
        HILOGE("Failed to chang mtime of the file for %{public}d", ret);
        return GetErrorCode(ret);
    }
    return SUCCESS_CODE;
}

std::tuple<int32_t, sptr<WatcherImpl>> FileFsImpl::CreateWatcher(std::string path, uint32_t events,
    void (*callback)(CWatchEvent))
{
    std::shared_ptr<WatcherInfoArg> infoArg = std::make_shared<WatcherInfoArg>(callback);
    if (!FileWatcherManager::GetInstance().CheckEventValid(events)) {
        return { GetErrorCode(EINVAL), nullptr };
    }
    infoArg->events = events;
    infoArg->fileName = path;

    auto watcherImpl = FFIData::Create<WatcherImpl>();
    if (!watcherImpl) {
        return {GetErrorCode(ENOMEM), nullptr};
    }
    watcherImpl->data_ = infoArg;

    if (FileWatcherManager::GetInstance().GetNotifyId() < 0 &&
        !FileWatcherManager::GetInstance().InitNotify()) {
        HILOGE("Failed to get notifyId or initnotify fail");
        return { GetErrorCode(errno), nullptr };
    }

    bool ret = FileWatcherManager::GetInstance().AddWatcherInfo(infoArg->fileName, infoArg);
    if (!ret) {
        HILOGE("Failed to add watcher info.");
        return {GetErrorCode(EINVAL), nullptr};
    }
    return {SUCCESS_CODE, watcherImpl};
}

} // CJSystemapi
} // namespace OHOS