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

#include "copydir_core.h"

#include <dirent.h>
#include <filesystem>
#include <memory>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static int RecurCopyDir(
    const string &srcPath, const string &destPath, const int mode, vector<struct ConflictFiles> &errfiles);

static bool EndWithSlash(const string &src)
{
    return src.back() == '/';
}

static bool AllowToCopy(const string &src, const string &dest)
{
    if (src == dest) {
        HILOGE("Failed to copy file, the same path");
        return false;
    }
    if (EndWithSlash(src) ? dest.find(src) == 0 : dest.find(src + "/") == 0) {
        HILOGE("Failed to copy file, dest is under src");
        return false;
    }
    if (filesystem::path(src).parent_path() == dest) {
        HILOGE("Failed to copy file, src's parent path is dest");
        return false;
    }
    return true;
}

static tuple<bool, int32_t> ParseAndCheckOperand(const string &src, const string &dest, const optional<int32_t> &mode)
{
    error_code errCode;
    if (!filesystem::is_directory(filesystem::status(src, errCode))) {
        HILOGE("Invalid src, errCode = %{public}d", errCode.value());
        return { false, 0 };
    }
    if (!filesystem::is_directory(filesystem::status(dest, errCode))) {
        HILOGE("Invalid dest, errCode = %{public}d", errCode.value());
        return { false, 0 };
    }
    if (!AllowToCopy(src, dest)) {
        return { false, 0 };
    }
    int32_t modeValue = 0;
    if (mode.has_value()) {
        modeValue = mode.value();
        if (modeValue < COPYMODE_MIN || modeValue > COPYMODE_MAX) {
            HILOGE("Invalid mode");
            return { false, 0 };
        }
    }
    return { true, modeValue };
}

static int MakeDir(const string &path)
{
    filesystem::path destDir(path);
    error_code errCode;
    if (!filesystem::create_directory(destDir, errCode)) {
        HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

struct NameList {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

static int RemoveFile(const string &destPath)
{
    filesystem::path destFile(destPath);
    error_code errCode;
    if (!filesystem::remove(destFile, errCode)) {
        HILOGE("Failed to remove file with path, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

static void Deleter(struct NameList *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int CopyFile(const string &src, const string &dest, const int mode)
{
    filesystem::path dstPath(dest);
    error_code errCode;
    if (filesystem::exists(dstPath, errCode)) {
        int ret = (mode == DIRMODE_FILE_COPY_THROW_ERR) ? EEXIST : RemoveFile(dest);
        if (ret) {
            HILOGE("Failed to copy file due to existing destPath with throw err");
            return ret;
        }
    }
    if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    filesystem::path srcPath(src);
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

static int CopySubDir(
    const string &srcPath, const string &destPath, const int mode, vector<struct ConflictFiles> &errfiles)
{
    error_code errCode;
    if (!filesystem::exists(destPath, errCode) && errCode.value() == ERRNO_NOERR) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    } else if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    return RecurCopyDir(srcPath, destPath, mode, errfiles);
}

static int FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return DISMATCH;
    }
    return MATCH;
}

static int RecurCopyDir(
    const string &srcPath, const string &destPath, const int mode, vector<struct ConflictFiles> &errfiles)
{
    unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    if (num < 0) {
        HILOGE("scandir fail errno is %{public}d", errno);
        return errno;
    }
    pNameList->direntNum = num;

    for (int i = 0; i < num; i++) {
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            string srcTemp = srcPath + '/' + string((pNameList->namelist[i])->d_name);
            string destTemp = destPath + '/' + string((pNameList->namelist[i])->d_name);
            int res = CopySubDir(srcTemp, destTemp, mode, errfiles);
            if (res == ERRNO_NOERR) {
                continue;
            }
            return res;
        } else {
            string src = srcPath + '/' + string((pNameList->namelist[i])->d_name);
            string dest = destPath + '/' + string((pNameList->namelist[i])->d_name);
            int res = CopyFile(src, dest, mode);
            if (res == EEXIST) {
                errfiles.emplace_back(src, dest);
                continue;
            } else if (res == ERRNO_NOERR) {
                continue;
            } else {
                HILOGE("Failed to copy file for error %{public}d", res);
                return res;
            }
        }
    }
    return ERRNO_NOERR;
}

static int CopyDirFunc(const string &src, const string &dest, const int mode, vector<struct ConflictFiles> &errfiles)
{
    size_t found = string(src).rfind('/');
    if (found == string::npos) {
        return EINVAL;
    }
    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    error_code errCode;
    if (!filesystem::exists(destStr, errCode) && errCode.value() == ERRNO_NOERR) {
        int res = MakeDir(destStr);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    } else if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    int res = RecurCopyDir(src, destStr, mode, errfiles);
    if (!errfiles.empty() && res == ERRNO_NOERR) {
        return EEXIST;
    }
    return res;
}

struct CopyDirResult CopyDirCore::DoCopyDir(const string &src, const string &dest, const optional<int32_t> &mode)
{
    auto [succ, modeValue] = ParseAndCheckOperand(src, dest, mode);
    if (!succ) {
        return { FsResult<void>::Error(EINVAL), nullopt };
    }

    vector<struct ConflictFiles> errfiles = {};
    int ret = CopyDirFunc(src, dest, modeValue, errfiles);
    if (ret == EEXIST && modeValue == DIRMODE_FILE_COPY_THROW_ERR) {
        return { FsResult<void>::Error(EEXIST), make_optional<vector<struct ConflictFiles>>(move(errfiles)) };
    } else if (ret) {
        return { FsResult<void>::Error(ret), nullopt };
    }
    return { FsResult<void>::Success(), nullopt };
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS