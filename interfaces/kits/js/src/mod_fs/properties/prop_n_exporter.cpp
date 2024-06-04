/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "prop_n_exporter.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "common_func.h"
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "close.h"
#include "fdatasync.h"
#include "file_utils.h"
#include "filemgmt_libn.h"
#include "fsync.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "lstat.h"
#include "mkdtemp.h"
#include "open.h"
#include "rename.h"
#include "rmdirent.h"
#include "stat.h"
#include "truncate.h"
#include "utimes.h"

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "copy.h"
#include "copy_file.h"
#include "copydir.h"
#include "create_randomaccessfile.h"
#include "create_stream.h"
#include "dup.h"
#include "fdopen_stream.h"
#include "listfile.h"
#include "lseek.h"
#include "move.h"
#include "movedir.h"
#include "read_lines.h"
#include "read_text.h"
#include "rust_file.h"
#include "symlink.h"
#include "watcher.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static int AccessCore(const string &path, int mode)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> access_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!access_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_access(nullptr, access_req.get(), path.c_str(), mode, nullptr);
    return ret;
}

static int GetMode(NVal secondVar, bool *hasMode)
{
    if (secondVar.TypeIs(napi_number)) {
        bool succ = false;
        int mode = 0;
        *hasMode = true;
        tie(succ, mode) = secondVar.ToInt32();
        if (succ && (static_cast<unsigned int>(mode) & 0x06) == static_cast<unsigned int>(mode)) {
            return mode;
        }
    }

    return -1;
}

napi_value PropNExporter::AccessSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool hasMode = false;
    int mode = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        mode = GetMode(NVal(env, funcArg[NARG_POS::SECOND]), &hasMode);
    }
    if (mode < 0 && hasMode) {
        HILOGE("Invalid mode from JS second argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool isAccess = false;
    int ret = AccessCore(path.get(), mode);
    if (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) {
        HILOGE("Failed to access file by path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    if (ret == 0) {
        isAccess = true;
    }
    return NVal::CreateBool(env, isAccess).val_;
}

napi_value PropNExporter::Access(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool hasMode = false;
    int mode = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        mode = GetMode(NVal(env, funcArg[NARG_POS::SECOND]), &hasMode);
    }
    if (mode < 0 && hasMode) {
        HILOGE("Invalid mode from JS second argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    mode = hasMode ? mode : 0;

    auto result = CreateSharedPtr<AsyncAccessArg>();
    if (result == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [path = string(tmp.get()), result, mode]() -> NError {
        int ret = AccessCore(path, mode);
        if (ret == 0) {
            result->isAccess = true;
        }
        return (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) ? NError(ret) : NError(ERRNO_NOERR);
    };

    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateBool(env, result->isAccess);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || hasMode) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_ACCESS_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_ACCESS_NAME, cbExec, cbComplete).val_;
    }
}

napi_value PropNExporter::Unlink(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of Arguments Unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [path = string(tmp.get())]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> unlink_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!unlink_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_unlink(nullptr, unlink_req.get(), path.c_str(), nullptr);
        if (ret < 0) {
            HILOGD("Failed to unlink with path");
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_UNLINK_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_UNLINK_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::UnlinkSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> unlink_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!unlink_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_unlink(nullptr, unlink_req.get(), path.get(), nullptr);
    if (ret < 0) {
        HILOGD("Failed to unlink with path");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

static int MkdirCore(const string &path)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> mkdir_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!mkdir_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_mkdir(nullptr, mkdir_req.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
}

static NError MkdirExec(const string &path, bool recursion, bool hasOption)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (hasOption) {
        int ret = AccessCore(path, 0);
        if (ret == ERRNO_NOERR) {
            HILOGE("The path already exists");
            return NError(EEXIST);
        }
        if (ret != -ENOENT) {
            HILOGE("Failed to check for illegal path or request for heap memory");
            return NError(ret);
        }
        if (::Mkdirs(path.c_str(), static_cast<MakeDirectionMode>(recursion)) < 0) {
            HILOGE("Failed to create directories, error: %{public}d", errno);
            return NError(errno);
        }
        ret = AccessCore(path, 0);
        if (ret) {
            HILOGE("Failed to verify the result of Mkdirs function");
            return NError(ret);
        }
        return NError(ERRNO_NOERR);
    }
#endif
    int ret = MkdirCore(path);
    if (ret) {
        HILOGD("Failed to create directory");
        return NError(ret);
    }
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Mkdir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool recursion = false;
    bool hasOption = false;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(hasOption, recursion) = option.ToBool(false);
            if (!hasOption) {
                NError(EINVAL).ThrowErr(env);
                return nullptr;
            }
        }
    }
#endif
    auto cbExec = [path = string(tmp.get()), recursion, hasOption]() -> NError {
        return MkdirExec(path, recursion, hasOption);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_MKDIR_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[funcArg.GetArgc() - 1]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_MKDIR_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::MkdirSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool hasOption = false;
    bool recursion = false;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(hasOption, recursion) = NVal(env, funcArg[NARG_POS::SECOND]).ToBool(false);
        if (!hasOption) {
            HILOGE("Invalid recursion mode");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
#endif
    auto err = MkdirExec(path.get(), recursion, hasOption);
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    tie(succ, buf, len, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(ret)).val_;
}

static NError ReadExec(shared_ptr<AsyncIOReadArg> arg, char *buf, size_t len, int32_t fd, int64_t offset)
{
    uv_buf_t buffer = uv_buf_init(buf, len);
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        return NError(ret);
    }
    arg->lenRead = ret;
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int32_t fd = 0;
    int64_t offset = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    tie(succ, buf, len, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf, len, fd, offset]() -> NError {
        return ReadExec(arg, static_cast<char *>(buf), len, fd, offset);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, static_cast<int64_t>(arg->lenRead)) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READ_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READ_NAME, cbExec, cbCompl).val_;
    }
}

static NError WriteExec(shared_ptr<AsyncIOWrtieArg> arg, char *buf, size_t len, int32_t fd, int64_t offset)
{
    uv_buf_t buffer = uv_buf_init(buf, static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> write_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!write_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        return NError(ret);
    }
    arg->actLen = ret;
    return NError(ERRNO_NOERR);
}

napi_value PropNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto[succ, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<char[]> bufGuard = nullptr;
    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    tie(succ, bufGuard, buf, len, offset) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<AsyncIOWrtieArg>(move(bufGuard));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, buf, len, fd = fd, offset]() -> NError {
        return WriteExec(arg, static_cast<char *>(buf), len, fd, offset);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, static_cast<int64_t>(arg->actLen)) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_WRITE_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_WRITE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    int32_t fd = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t offset = -1;
    unique_ptr<char[]> bufGuard = nullptr;
    tie(succ, bufGuard, buf, len, offset) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> write_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!write_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, static_cast<int64_t>(ret)).val_;
}
bool PropNExporter::ExportSync()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("accessSync", AccessSync),
        NVal::DeclareNapiFunction("closeSync", Close::Sync),
        NVal::DeclareNapiFunction("fdatasyncSync", Fdatasync::Sync),
        NVal::DeclareNapiFunction("fsyncSync", Fsync::Sync),
        NVal::DeclareNapiFunction("lstatSync", Lstat::Sync),
        NVal::DeclareNapiFunction("mkdirSync", MkdirSync),
        NVal::DeclareNapiFunction("mkdtempSync", Mkdtemp::Sync),
        NVal::DeclareNapiFunction("openSync", Open::Sync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("renameSync", Rename::Sync),
        NVal::DeclareNapiFunction("rmdirSync", Rmdirent::Sync),
        NVal::DeclareNapiFunction("statSync", Stat::Sync),
        NVal::DeclareNapiFunction("truncateSync", Truncate::Sync),
        NVal::DeclareNapiFunction("unlinkSync", UnlinkSync),
        NVal::DeclareNapiFunction("utimes", Utimes::Sync),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        NVal::DeclareNapiFunction("copyDirSync", CopyDir::Sync),
        NVal::DeclareNapiFunction("copyFileSync", CopyFile::Sync),
        NVal::DeclareNapiFunction("createRandomAccessFileSync", CreateRandomAccessFile::Sync),
        NVal::DeclareNapiFunction("createStreamSync", CreateStream::Sync),
        NVal::DeclareNapiFunction("dup", Dup::Sync),
        NVal::DeclareNapiFunction("fdopenStreamSync", FdopenStream::Sync),
        NVal::DeclareNapiFunction("listFileSync", ListFile::Sync),
        NVal::DeclareNapiFunction("lseek", Lseek::Sync),
        NVal::DeclareNapiFunction("moveDirSync", MoveDir::Sync),
        NVal::DeclareNapiFunction("moveFileSync", Move::Sync),
        NVal::DeclareNapiFunction("readLinesSync", ReadLines::Sync),
        NVal::DeclareNapiFunction("readTextSync", ReadText::Sync),
        NVal::DeclareNapiFunction("symlinkSync", Symlink::Sync),
#endif
    });
}

bool PropNExporter::ExportAsync()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("access", Access),
        NVal::DeclareNapiFunction("close", Close::Async),
        NVal::DeclareNapiFunction("fdatasync", Fdatasync::Async),
        NVal::DeclareNapiFunction("fsync", Fsync::Async),
        NVal::DeclareNapiFunction("lstat", Lstat::Async),
        NVal::DeclareNapiFunction("mkdir", Mkdir),
        NVal::DeclareNapiFunction("mkdtemp", Mkdtemp::Async),
        NVal::DeclareNapiFunction("open", Open::Async),
        NVal::DeclareNapiFunction("rename", Rename::Async),
        NVal::DeclareNapiFunction("rmdir", Rmdirent::Async),
        NVal::DeclareNapiFunction("stat", Stat::Async),
        NVal::DeclareNapiFunction("truncate", Truncate::Async),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("unlink", Unlink),
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        NVal::DeclareNapiFunction("copyDir", CopyDir::Async),
        NVal::DeclareNapiFunction("copyFile", CopyFile::Async),
        NVal::DeclareNapiFunction("copy", Copy::Async),
        NVal::DeclareNapiFunction("createRandomAccessFile", CreateRandomAccessFile::Async),
        NVal::DeclareNapiFunction("createStream", CreateStream::Async),
        NVal::DeclareNapiFunction("fdopenStream", FdopenStream::Async),
        NVal::DeclareNapiFunction("listFile", ListFile::Async),
        NVal::DeclareNapiFunction("moveDir", MoveDir::Async),
        NVal::DeclareNapiFunction("moveFile", Move::Async),
        NVal::DeclareNapiFunction("readLines", ReadLines::Async),
        NVal::DeclareNapiFunction("readText", ReadText::Async),
        NVal::DeclareNapiFunction("symlink", Symlink::Async),
        NVal::DeclareNapiFunction("createWatcher", Watcher::CreateWatcher),
#endif
    });
}

bool PropNExporter::Export()
{
    return ExportSync() && ExportAsync();
}

#ifdef WIN_PLATFORM
string PropNExporter::GetNExporterName()
#else
string PropNExporter::GetClassName()
#endif
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporter::~PropNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS