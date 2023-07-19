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

#include "copydir.h"

#include <dirent.h>
#include <filesystem>
#include <memory>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static int RecurCopyDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ConflictFiles> &errfiles);

static bool AllowToCopy(const string &src, const string &dest)
{
    if (dest.find(src) == 0 || filesystem::path(src).parent_path() == dest) {
        HILOGE("Failed to copy file");
        return false;
    }
    return true;
}

static tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>, int> ParseAndCheckJsOperand(napi_env env,
    const NFuncArg &funcArg)
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
    if (!AllowToCopy(src.get(), dest.get())) {
        return { false, nullptr, nullptr, 0 };
    }
    int mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        bool resGetThirdArg = false;
        tie(resGetThirdArg, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!resGetThirdArg || (mode < COPYMODE_MIN || mode > COPYMODE_MAX)) {
            HILOGE("Invalid mode");
            return { false, nullptr, nullptr, 0 };
        }
    }
    return { true, move(src), move(dest), mode };
}

static int MakeDir(const string &path)
{
    filesystem::path destDir(path);
    std::error_code errCode;
    if (!filesystem::create_directory(destDir, errCode)) {
        HILOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

struct NameList {
    struct dirent** namelist = { nullptr };
    int direntNum = 0;
};

static int RemoveFile(const string& destPath)
{
    filesystem::path destFile(destPath);
    std::error_code errCode;
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
}

static int CopyFile(const string &src, const string &dest, const int mode)
{
    filesystem::path dstPath(dest);
    if (filesystem::exists(dstPath)) {
        int ret = (mode == DIRMODE_FILE_COPY_THROW_ERR) ? EEXIST : RemoveFile(dest);
        if (ret) {
            HILOGE("Failed to copy file due to existing destPath with throw err");
            return ret;
        }
    }
    filesystem::path srcPath(src);
    std::error_code errCode;
    if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
        HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return ERRNO_NOERR;
}

static int CopySubDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ConflictFiles> &errfiles)
{
    if (!filesystem::exists(destPath)) {
        int res = MakeDir(destPath);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
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

static int RecurCopyDir(const string &srcPath, const string &destPath, const int mode,
    vector<struct ConflictFiles> &errfiles)
{
    unique_ptr<struct NameList, decltype(Deleter)*> pNameList = {new (nothrow) struct NameList, Deleter};
    if (pNameList == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(srcPath.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
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
    if (found == std::string::npos) {
        return EINVAL;
    }
    string dirName = string(src).substr(found);
    string destStr = dest + dirName;
    if (!filesystem::exists(destStr)) {
        int res = MakeDir(destStr);
        if (res != ERRNO_NOERR) {
            HILOGE("Failed to mkdir");
            return res;
        }
    }
    int res = RecurCopyDir(src, destStr, mode, errfiles);
    if (!errfiles.empty() && res == ERRNO_NOERR) {
        return EEXIST;
    }
    return res;
}

static napi_value PushErrFilesInData(napi_env env, vector<struct ConflictFiles> &errfiles)
{
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to creat array");
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

napi_value CopyDir::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseAndCheckJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    vector<struct ConflictFiles> errfiles = {};
    int ret = CopyDirFunc(src.get(), dest.get(), mode, errfiles);
    if (ret == EEXIST && mode == DIRMODE_FILE_COPY_THROW_ERR) {
        NError(ret).ThrowErrAddData(env, EEXIST, PushErrFilesInData(env, errfiles));
        return nullptr;
    } else if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

struct CopyDirArgs {
    vector<ConflictFiles> errfiles;
    int errNo = ERRNO_NOERR;
};

napi_value CopyDir::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, src, dest, mode] = ParseAndCheckJsOperand(env, funcArg);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<CopyDirArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [srcPath = string(src.get()), destPath = string(dest.get()), mode = mode, arg]() -> NError {
        arg->errNo = CopyDirFunc(srcPath, destPath, mode, arg->errfiles);
        if (arg->errNo) {
            return NError(arg->errNo);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplCallback = [arg, mode = mode](napi_env env, NError err) -> NVal {
        if (arg->errNo == EEXIST && mode == DIRMODE_FILE_COPY_THROW_ERR) {
            napi_value data = err.GetNapiErr(env);
            napi_status status = napi_set_named_property(env, data, FILEIO_TAG_ERR_DATA.c_str(),
                PushErrFilesInData(env, arg->errfiles));
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
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_COPYDIR_NAME, cbExec, cbComplCallback).val_;
    } else {
        int cbIdex = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdex]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_COPYDIR_NAME, cbExec, cbComplCallback).val_;
    }
}

} // ModuleFileIO
} // FileManagement
} // OHOS