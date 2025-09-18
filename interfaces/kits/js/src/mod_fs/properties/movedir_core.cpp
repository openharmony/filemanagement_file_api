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

#include "movedir_core.h"

#include <deque>
#include <dirent.h>
#include <filesystem>
#include <memory>
#include <string_view>
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ErrFiles> &errfiles);

static tuple<bool, bool> JudgeExistAndEmpty(const string &path)
{
    std::error_code errCode;
    filesystem::path pathName(path);
    if (filesystem::exists(pathName, errCode)) {
        if (filesystem::is_empty(pathName, errCode)) {
            return { true, true };
        }
        return { true, false };
    }
    return { false, false };
}

static int RmDirectory(const string &path)
{
    filesystem::path pathName(path);
    std::error_code existsErrCode;
    if (filesystem::exists(pathName, existsErrCode)) {
        std::error_code removeErrCode;
        (void)filesystem::remove_all(pathName, removeErrCode);
        if (removeErrCode.value() != 0) {
            HILOGE("Failed to remove directory, error code: %{public}d", removeErrCode.value());
            return removeErrCode.value();
        }
    } else if (existsErrCode.value() != ERRNO_NOERR) {
        HILOGE("Fs exists fail, errcode is %{public}d", existsErrCode.value());
        return existsErrCode.value();
    }
    return ERRNO_NOERR;
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

static int CopyAndDeleteFile(const string &src, const string &dest)
{
    filesystem::path dstPath(dest);
    std::error_code errCode;
    if (filesystem::exists(dstPath, errCode)) {
        int removeRes = RemovePath(dest);
        if (removeRes != 0) {
            HILOGE("Failed to remove dest file");
            return removeRes;
        }
    }
    filesystem::path srcPath(src);
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return RemovePath(src);
}

static int RenameFile(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    filesystem::path dstPath(dest);
    std::error_code errCode;
    if (filesystem::exists(dstPath, errCode)) {
        if (filesystem::is_directory(dstPath, errCode)) {
            errfiles.emplace_front(src, dest);
            return ERRNO_NOERR;
        }
        if (mode == DIRMODE_FILE_THROW_ERR) {
            errfiles.emplace_back(src, dest);
            return ERRNO_NOERR;
        }
    }
    if (errCode.value() != ERRNO_NOERR) {
        HILOGE("Fs exists or is_directory fail, errcode is %{public}d", errCode.value());
    }
    filesystem::path srcPath(src);
    filesystem::rename(srcPath, dstPath, errCode);
    if (errCode.value() == EXDEV) {
        HILOGD("Failed to rename file due to EXDEV");
        return CopyAndDeleteFile(src, dest);
    }
    return errCode.value();
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILE_DISMATCH;
    }
    return FILE_MATCH;
}

static int RenameDir(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    filesystem::path destPath(dest);
    std::error_code errCode;
    if (filesystem::exists(destPath, errCode)) {
        return RecurMoveDir(src, dest, mode, errfiles);
    } else if (errCode.value() != ERRNO_NOERR) {
        HILOGE("Fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    filesystem::path srcPath(src);
    filesystem::rename(srcPath, destPath, errCode);
    if (errCode.value() == EXDEV) {
        HILOGD("Failed to rename file due to EXDEV");
        if (!filesystem::create_directory(destPath, errCode)) {
            HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
            return errCode.value();
        }
        return RecurMoveDir(src, dest, mode, errfiles);
    }
    if (errCode.value() != 0) {
        HILOGE("Failed to rename file, error code: %{public}d", errCode.value());
        return errCode.value();
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
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    deque<struct ErrFiles> &errfiles)
{
    filesystem::path dpath(destPath);
    std::error_code errCode;
    if (!filesystem::is_directory(dpath, errCode)) {
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

static int MoveDirFunc(const string &src, const string &dest, const int mode, deque<struct ErrFiles> &errfiles)
{
    size_t found = string(src).rfind('/');
    if (found == std::string::npos) {
        return EINVAL;
    }
    if (access(src.c_str(), W_OK) != 0) {
        HILOGE("Failed to move src directory due to doesn't exist or hasn't write permission");
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

static tuple<bool, int> ValidMoveDirArg(
    const string &src, const string &dest, optional<int32_t> mode)
{
    std::error_code errCode;
    if (!filesystem::is_directory(filesystem::status(src.c_str(), errCode))) {
        HILOGE("Invalid src, errCode = %{public}d", errCode.value());
        return { false, 0 };
    }
    if (!filesystem::is_directory(filesystem::status(dest.c_str(), errCode))) {
        HILOGE("Invalid dest,errCode = %{public}d", errCode.value());
        return { false, 0 };
    }
    int modeType = 0;
    if (mode.has_value()) {
        modeType = mode.value();
        if (modeType < DIRMODE_MIN || modeType > DIRMODE_MAX) {
            HILOGE("Invalid mode");
            return { false, 0 };
        }
    }
    return { true, modeType };
}

MoveDirResult MoveDirCore::DoMoveDir(
    const string &src, const string &dest, optional<int32_t> modeType)
{
    auto [succ, mode] = ValidMoveDirArg(src, dest, modeType);
    if (!succ) {
        return { FsResult<void>::Error(EINVAL), nullopt };
    }
    deque<struct ErrFiles> errfiles = {};
    int ret = MoveDirFunc(src, dest, mode, errfiles);
    if (ret == EEXIST) {
        return { FsResult<void>::Error(EEXIST), optional<deque<struct ErrFiles>> { errfiles } };
    } else if (ret) {
        return { FsResult<void>::Error(ret), nullopt };
    }
    return { FsResult<void>::Success(), nullopt };
}

} // ModuleFileIO
} // FileManagement
} // OHOS