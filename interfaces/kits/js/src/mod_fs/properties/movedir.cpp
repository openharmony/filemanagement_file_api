/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "movedir.h"

#include <dirent.h>
#include <filesystem>
#include <memory>
#include <string_view>
#include <tuple>
#include <unistd.h>
#include <deque>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

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
    std::error_code errCode;
    if (filesystem::exists(pathName, errCode)) {
        std::error_code errCode;
        (void)filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            HILOGE("Failed to remove directory, error code: %{public}d", errCode.value());
            return errCode.value();
        }
    } else if (errCode.value() != ERRNO_NOERR) {
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
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

static tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>, int> ParseJsOperand(napi_env env, const NFuncArg& funcArg)
{
    auto [resGetFirstArg, src, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    std::error_code errCode;
    if (!resGetFirstArg || !filesystem::is_directory(filesystem::status(src.get(), errCode))) {
        HILOGE("Invalid src, errCode = %{public}d", errCode.value());
        return { false, nullptr, nullptr, 0 };
    }
    auto [resGetSecondArg, dest, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8StringPath();
    if (!resGetSecondArg || !filesystem::is_directory(filesystem::status(dest.get(), errCode))) {
        HILOGE("Invalid dest,errCode = %{public}d", errCode.value());
        return { false, nullptr, nullptr, 0 };
    }
    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        bool resGetThirdArg = false;
        tie(resGetThirdArg, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!resGetThirdArg || (mode < DIRMODE_MIN || mode > DIRMODE_MAX)) {
            HILOGE("Invalid mode");
            return { false, nullptr, nullptr, 0 };
        }
    }
    return { true, move(src), move(dest), mode };
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
        HILOGE("fs exists or is_directory fail, errcode is %{public}d", errCode.value());
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
        HILOGE("fs exists fail, errcode is %{public}d", errCode.value());
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
    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    auto [destStrExist, destStrEmpty] = JudgeExistAndEmpty(destStr);
    if (destStrExist && !destStrEmpty) {
        if (mode == DIRMODE_DIRECTORY_REPLACE) {
            int removeRes = RmDirectory(destStr);
            if (removeRes) {
                HILOGE("Failed to remove dest directory in DIRMODE_DIRECTORY_REPLACE, ret %{public}d", removeRes);
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
            HILOGE("Failed to remove src directory, ret %{public}d", removeRes);
            return removeRes;
        }
    }
    return res;
}

static napi_value GetErrData(napi_env env, deque<struct ErrFiles> &errfiles)
{
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to create array");
        return nullptr;
    }
    size_t index = 0;
    for (auto &iter : errfiles) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("srcFile", NVal::CreateUTF8String(env, iter.srcFiles).val_);
        obj.AddProp("destFile", NVal::CreateUTF8String(env, iter.destFiles).val_);
        status = napi_set_element(env, res, index++, obj.val_);
        if (status != napi_ok) {
            HILOGE("Failed to set element on data");
            return nullptr;
        }
    }
    return res;
}

napi_value MoveDir::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    deque<struct ErrFiles> errfiles = {};
    int ret = MoveDirFunc(src.get(), dest.get(), mode, errfiles);
    if (ret == EEXIST) {
        NError(ret).ThrowErrAddData(env, EEXIST, GetErrData(env, errfiles));
        return nullptr;
    } else if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

struct MoveDirArgs {
    deque<ErrFiles> errfiles;
    int errNo = 0;
    ~MoveDirArgs() = default;
};

napi_value MoveDir::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto arg = CreateSharedPtr<MoveDirArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [srcPath = string(src.get()), destPath = string(dest.get()), mode = mode, arg]() -> NError {
        arg->errNo = MoveDirFunc(srcPath, destPath, mode, arg->errfiles);
        if (arg->errNo) {
            return NError(arg->errNo);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [arg, mode = mode](napi_env env, NError err) -> NVal {
        if (arg->errNo == EEXIST) {
            napi_value data = err.GetNapiErr(env);
            napi_status status = napi_set_named_property(env, data, FILEIO_TAG_ERR_DATA.c_str(),
                GetErrData(env, arg->errfiles));
            if (status != napi_ok) {
                HILOGE("Failed to set data property on Error");
            }
            return { env, data };
        } else if (arg->errNo) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
            !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MOVEDIR_NAME, cbExec, cbComplCallback).val_;
    } else {
        int cbIdex = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdex]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MOVEDIR_NAME, cbExec, cbComplCallback).val_;
    }
}

} // ModuleFileIO
} // FileManagement
} // OHOS