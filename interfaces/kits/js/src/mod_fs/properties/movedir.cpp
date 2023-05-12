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
#include <vector>

#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ErrFiles> &errfiles);

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

static bool RmDirectory(const string &path)
{
    filesystem::path pathName(path);
    uintmax_t num = filesystem::remove_all(pathName);
    if (!num || filesystem::exists(pathName)) {
        HILOGE("Failed to rmdir path");
        return false;
    }
    return true;
}

static tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>, int> ParseJsOperand(napi_env env, const NFuncArg& funcArg)
{
    auto [resGetFirstArg, src, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg || !filesystem::is_directory(filesystem::status(src.get()))) {
        HILOGE("Invalid src");
        return { false, nullptr, nullptr, 0 };
    }
    auto [resGetSecondArg, dest, unused] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg || !filesystem::is_directory(filesystem::status(dest.get()))) {
        HILOGE("Invalid dest");
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
    uv_fs_t copyfile_req;
    int ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), UV_FS_COPYFILE_FICLONE_FORCE, nullptr);
    if (ret < 0) {
        HILOGE("Failed to move file using copyfile interface");
        return -ret;
    }
    uv_fs_req_cleanup(&copyfile_req);
    uv_fs_t unlink_req;
    int unlinkSrcRes = uv_fs_unlink(nullptr, &unlink_req, src.c_str(), nullptr);
    if (unlinkSrcRes < 0) {
        HILOGE("Failed to unlink src file because errno number is: %{public}d", unlinkSrcRes);
        int unlinkDestRes = uv_fs_unlink(nullptr, &unlink_req, dest.c_str(), nullptr);
        if (unlinkDestRes < 0) {
            HILOGE("Failed to unlink dest file because errno number is: %{public}d", unlinkDestRes);
        }
        uv_fs_req_cleanup(&unlink_req);
        return -unlinkSrcRes;
    }
    uv_fs_req_cleanup(&unlink_req);
    return ERRNO_NOERR;
}

static int RenameFile(const string &src, const string &dest, const int mode)
{
    if (mode == DIRMODE_FILE_THROW_ERR || mode == DIRMODE_DIRECTORY_THROW_ERR) {
        auto [fileExist, ignore] = JudgeExistAndEmpty(dest);
        if (fileExist) {
            HILOGE("Failed to move file due to existing destPath with throw err");
            return EEXIST;
        }
    }
    uv_fs_t rename_req;
    int res = uv_fs_rename(nullptr, &rename_req, src.c_str(), dest.c_str(), nullptr);
    uv_fs_req_cleanup(&rename_req);
    if (res < 0 && errno == EXDEV) {
        HILOGE("Failed to rename file due to EXDEV");
        return CopyAndDeleteFile(src, dest);
    }
    if (res < 0) {
        HILOGE("Failed to move file using rename syscall");
        return -res;
    }
    return ERRNO_NOERR;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILE_DISMATCH;
    }
    return FILE_MATCH;
}

static int RenameDir(const string &src, const string &dest, const int mode, vector<struct ErrFiles> &errfiles)
{
    uv_fs_t rename_req;
    int ret = uv_fs_rename(nullptr, &rename_req, src.c_str(), dest.c_str(), nullptr);
    uv_fs_req_cleanup(&rename_req);
    if (ret < 0 && errno == EXDEV) {
        HILOGE("Failed to rename file due to EXDEV");
        return RecurMoveDir(src, dest, mode, errfiles);
    }
    if (ret < 0) {
        HILOGE("Failed to move file using rename syscall");
        return -ret;
    }
    return ERRNO_NOERR;
}

struct NameListArg {
    struct dirent** namelist;
    int num;
};

static void Deletor(struct NameListArg *arg)
{
    for (int i = 0; i < arg->num; i++) {
        delete (arg->namelist)[i];
        (arg->namelist)[i] = nullptr;
    }
}

static int MoveSubDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ErrFiles> &errfiles)
{
    auto [fileExist, ignore] = JudgeExistAndEmpty(destPath);
    if (fileExist) {
        return RecurMoveDir(srcPath, destPath, mode, errfiles);
    } else {
        return RenameDir(srcPath, destPath, mode, errfiles);
    }
}

static int RecurMoveDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ErrFiles> &errfiles)
{
    unique_ptr<struct NameListArg, decltype(Deletor)*> ptr = {new struct NameListArg, Deletor};
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
            int res = MoveSubDir(srcTemp, destTemp, mode, errfiles);
            if (res == ERRNO_NOERR) {
                continue;
            }
            return res;
        } else {
            string src = srcPath + '/' + string((ptr->namelist[i])->d_name);
            string dest = destPath + '/' + string((ptr->namelist[i])->d_name);
            int res = RenameFile(src, dest, mode);
            if (res == EEXIST && mode == DIRMODE_FILE_THROW_ERR) {
                errfiles.emplace_back(src, dest);
                continue;
            } else if (res == ERRNO_NOERR) {
                continue;
            } else {
                HILOGE("Failed to rename file");
                return res;
            }
        }
    }
    return ERRNO_NOERR;
}

static int MoveDirFunc(const string &src, const string &dest, const int mode, vector<struct ErrFiles> &errfiles)
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
            if (!RmDirectory(destStr)) {
                return ENOTEMPTY;
            }
            return RenameDir(src, destStr, mode, errfiles);
        }
        if (mode == DIRMODE_DIRECTORY_THROW_ERR) {
            return ENOTEMPTY;
        }
        int res = RecurMoveDir(src, destStr, mode, errfiles);
        if (res == ERRNO_NOERR && mode == DIRMODE_FILE_REPLACE) {
            if (!RmDirectory(src)) {
                HILOGE("Failed to rm src dir with DIRMODE_FILE_REPLACE");
                return ENOTEMPTY;
            }
        } else if (mode == DIRMODE_FILE_THROW_ERR) {
            HILOGE("Failed to movedir with some conflicted files");
            return EEXIST;
        }
        return res;
    } else {
        return RenameDir(src, destStr, mode, errfiles);
    }
    return ERRNO_NOERR;
}

static napi_value GetErrData(napi_env env, vector<struct ErrFiles> &errfiles)
{
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to create array");
        return nullptr;
    }
    for (size_t i = 0; i < errfiles.size(); i++) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("srcFile", NVal::CreateUTF8String(env, errfiles[i].srcFiles).val_);
        obj.AddProp("destFile", NVal::CreateUTF8String(env, errfiles[i].destFiles).val_);
        status = napi_set_element(env, res, i, obj.val_);
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

    vector<struct ErrFiles> errfiles = {};
    int ret = MoveDirFunc(src.get(), dest.get(), mode, errfiles);
    if (ret == EEXIST && mode == DIRMODE_FILE_THROW_ERR) {
        NError(ret).ThrowErrAddData(env, EEXIST, GetErrData(env, errfiles));
        return nullptr;
    } else if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

struct MoveDirArgs {
    vector<ErrFiles> errfiles;
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
    shared_ptr<MoveDirArgs> arg = CreateSharedPtr<MoveDirArgs>();
    if (arg == nullptr) {
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
        if (arg->errNo == EEXIST && mode == DIRMODE_FILE_THROW_ERR) {
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