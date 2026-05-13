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
#include "n_error.h"
#include "securec.h"
#include "copy_dir.h"
#include "utils.h"

using namespace OHOS;
using namespace OHOS::FFI;
using namespace OHOS::FileManagement;
using namespace OHOS::CJSystemapi;
using namespace OHOS::FileManagement::LibN;

namespace {

static int RecurCopyDir(const std::string &srcPath, const std::string &destPath, const int mode,
    std::vector<struct ConflictFiles> &errfiles);

static int RemoveFile(const std::string &destPath)
{
    std::filesystem::path destFile(destPath);
    std::error_code errCode;
    if (!std::filesystem::remove(destFile, errCode)) {
        LOGE("Failed to remove file with path, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static int CopyFile(const std::string &src, const std::string &dest, const int mode)
{
    std::filesystem::path dstPath(dest);
    if (std::filesystem::exists(dstPath)) {
        int ret = (mode == DIRMODE_FILE_COPY_THROW_ERR) ? EEXIST : RemoveFile(dest);
        if (ret) {
            LOGE("Failed to copy file due to existing destPath with throw err");
            return ret;
        }
    }
    std::filesystem::path srcPath(src);
    std::error_code errCode;
    if (!std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing, errCode)) {
        LOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static int CopySubDir(const std::string &srcPath, const std::string &destPath, const int mode,
    std::vector<struct ConflictFiles> &errfiles)
{
    if (!std::filesystem::exists(destPath)) {
        int res = OHOS::CJSystemapi::FileFs::CommonMakeDir(destPath);
        if (res != OHOS::FileManagement::LibN::ERRNO_NOERR) {
            LOGE("Failed to mkdir");
            return res;
        }
    }
    return RecurCopyDir(srcPath, destPath, mode, errfiles);
}

static int RecurCopyDir(const std::string &srcPath, const std::string &destPath, const int mode,
    std::vector<struct ConflictFiles> &errfiles)
{
    using OHOS::CJSystemapi::FileFs::NameListArg;
    using OHOS::CJSystemapi::FileFs::NameListArgDeleter;
    using OHOS::CJSystemapi::FileFs::CommonFilterFunc;

    std::unique_ptr<NameListArg, decltype(NameListArgDeleter)*> pNameList = {
        new (std::nothrow) NameListArg, NameListArgDeleter };
    if (pNameList == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), CommonFilterFunc, alphasort);
    pNameList->direntNum = num;

    for (int i = 0; i < num; i++) {
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            std::string srcTemp = srcPath + '/' + std::string((pNameList->namelist[i])->d_name);
            std::string destTemp = destPath + '/' + std::string((pNameList->namelist[i])->d_name);
            LOGI("srcTemp %{public}s from", srcTemp.c_str());
            LOGI("destTemp %{public}s to", destTemp.c_str());
            int res = CopySubDir(srcTemp, destTemp, mode, errfiles);
            if (res == OHOS::FileManagement::LibN::ERRNO_NOERR) {
                continue;
            }
            return res;
        } else {
            std::string src = srcPath + '/' + std::string((pNameList->namelist[i])->d_name);
            std::string dest = destPath + '/' + std::string((pNameList->namelist[i])->d_name);
            int res = CopyFile(src, dest, mode);
            if (res == EEXIST) {
                errfiles.emplace_back(src, dest);
                continue;
            } else if (res == OHOS::FileManagement::LibN::ERRNO_NOERR) {
                continue;
            } else {
                LOGE("Failed to copy file for error %{public}d", res);
                return res;
            }
        }
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static bool AllowToCopy(const std::string& src, const std::string& dest)
{
    if (dest.find(src) == 0 || std::filesystem::path(src).parent_path() == dest) {
        return false;
    }
    return true;
}

static int CopyDirFunc(const std::string &src, const std::string &dest, const int mode,
    std::vector<struct ConflictFiles> &errfiles)
{
    size_t found = std::string(src).rfind('/');
    if (found == std::string::npos) {
        LOGE("CopyDirFunc EINVAL");
        return EINVAL;
    }
    std::string dirName = std::string(src).substr(found);
    std::string destStr = dest + dirName;
    LOGI("destStr: %{public}s", destStr.c_str());
    if (!std::filesystem::exists(destStr)) {
        int res = OHOS::CJSystemapi::FileFs::CommonMakeDir(destStr);
        if (res != OHOS::FileManagement::LibN::ERRNO_NOERR) {
            LOGE("Failed to mkdir");
            return res;
        }
    }
    int res = RecurCopyDir(src, destStr, mode, errfiles);
    if (!errfiles.empty() && res == OHOS::FileManagement::LibN::ERRNO_NOERR) {
        LOGE("CopyDirFunc EEXIST");
        return EEXIST;
    }
    return res;
}

}

namespace OHOS {
namespace CJSystemapi {

RetDataCArrConflictFiles CopyDirImpl::CopyDir(const std::string& src, const std::string& dest, int mode)
{
    std::error_code errCode;
    RetDataCArrConflictFiles ret = { .code = EINVAL, .data = { .head = nullptr, .size = 0 } };
    if (!std::filesystem::is_directory(std::filesystem::status(dest, errCode))) {
        LOGE("Invalid dest, errCode = %{public}d", errCode.value());
        return ret;
    }
    if (!AllowToCopy(src, dest)) {
        LOGE("Failed to copy file");
        return ret;
    }
    if (mode < COPYMODE_MIN || mode > COPYMODE_MAX) {
        LOGE("Invalid mode");
        return ret;
    }
    std::vector<struct ConflictFiles> errfiles = {};
    ret.code = CopyDirFunc(src, dest, mode, errfiles);
    ret.data.size = static_cast<int64_t>(errfiles.size());
    ret.data.head = OHOS::CJSystemapi::FileFs::VectorToCConflict(errfiles);
    return ret;
}

}
} // namespace OHOS::CJSystemapi