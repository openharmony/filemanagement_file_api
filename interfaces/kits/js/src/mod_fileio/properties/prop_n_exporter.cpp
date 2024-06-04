/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "../common_func.h"
#include "chmod.h"
#include "chown.h"
#include "close.h"
#include "copy_file.h"
#include "create_stream.h"
#include "fchmod.h"
#include "fchown.h"
#include "fdatasync.h"
#include "fdopen_stream.h"
#include "fstat.h"
#include "fsync.h"
#include "ftruncate.h"
#include "hash.h"
#include "lchown.h"
#include "link.h"
#include "lseek.h"
#include "lstat.h"
#include "mkdtemp.h"
#include "open.h"
#include "open_dir.h"
#include "posix_fallocate.h"
#include "read_dir.h"
#include "read_text.h"
#include "rename.h"
#include "rmdir.h"
#include "rmdirent.h"
#include "stat.h"
#include "symlink.h"
#include "truncate.h"
#include "watcher.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;
namespace {
    static constexpr int MODE_RUO_RWX = 0770;
}

napi_value PropNExporter::AccessSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int32_t mode = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(mode);
        if (!succ || mode < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    int ret = access(path.get(), mode);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::Access(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int32_t mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(mode);
        if (!succ || mode < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }

    auto cbExec = [path = string(path.get()), mode](napi_env env) -> UniError {
        int ret = access(path.c_str(), mode);
        if (ret == -1) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbComplete = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    const string procedureName = "FileIOAccess";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        int cbInd = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbInd]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value PropNExporter::Unlink(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of Arguments Unmatched");
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto cbExec = [path = string(path.get())](napi_env env) -> UniError {
        if (unlink(path.c_str()) == -1) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    const string procedureName = "FileIOStreamUnlink";
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::Mkdir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int32_t mode = MODE_RUO_RWX;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        bool succGetMode = false;
        tie(succGetMode, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(mode);
        if (!succGetMode || mode < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    auto cbExec = [path = string(path.get()), mode](napi_env env) -> UniError {
        if (mkdir(path.c_str(), mode) == -1) {
            return UniError(errno);
        }
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    const string procedureName = "fileioMkdir";
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::MkdirSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int32_t mode = MODE_RUO_RWX;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        bool succ = false;
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(mode);
        if (!succ || mode < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    int ret = mkdir(path.get(), mode);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FchmodSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int mode = 0;
    tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    int ret = fchmod(fd, mode);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FchownSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int owner;
    tie(succ, owner) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid owner");
        return nullptr;
    }

    int group;
    tie(succ, group) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid group");
        return nullptr;
    }

    int ret = fchown(fd, owner, group);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t pos = -1;
    tie(succ, buf, len, pos, ignore) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t actLen = 0;
    if (pos >= 0) {
        actLen = pread(fd, buf, len, pos);
    } else {
        actLen = read(fd, buf, len);
    }

    if (actLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, actLen).val_;
}

struct AsyncIOReadArg {
    ssize_t lenRead { 0 };
    int64_t offset { 0 };
    NRef refReadBuf;

    explicit AsyncIOReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncIOReadArg() = default;
};

static UniError ReadCallbackExecutor(napi_env env, ssize_t actLen, shared_ptr<AsyncIOReadArg> arg, int64_t offset)
{
    if (actLen == -1) {
        return UniError(errno);
    } else {
        arg->lenRead = actLen;
        arg->offset = offset;
        return UniError(ERRNO_NOERR);
    }
}

static NVal ReadCallbackCompleter(napi_env env, UniError err, shared_ptr<AsyncIOReadArg> arg)
{
    if (err) {
        return { env, err.GetNapiErr(env) };
    }
    NVal obj = NVal::CreateObject(env);
    obj.AddProp({
        NVal::DeclareNapiProperty("bytesRead", NVal::CreateInt64(env, arg->lenRead).val_),
        NVal::DeclareNapiProperty("buffer", arg->refReadBuf.Deref(env).val_),
        NVal::DeclareNapiProperty("offset", NVal::CreateInt64(env, arg->offset).val_)
    });
    return { obj };
}

napi_value PropNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    size_t len = 0;
    int fd = -1;
    int64_t pos = -1;
    int64_t offset = 0;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    tie(succ, buf, len, pos, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    auto cbExec = [arg, buf, len, fd, pos, offset](napi_env env) -> UniError {
        ssize_t actLen = 0;
        if (pos >= 0) {
            actLen = pread(fd, buf, len, pos);
        } else {
            actLen = read(fd, buf, len);
        }
        return ReadCallbackExecutor(env, actLen, arg, offset);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        return ReadCallbackCompleter(env, err, arg);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIORead", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIORead", cbExec, cbCompl).val_;
    }
}

static UniError WriteExec(shared_ptr<AsyncIOWrtieArg> arg, void *buf, size_t len, int fd, int64_t position)
{
    if (position >= 0) {
        arg->actLen = pwrite(fd, buf, len, position);
    } else {
        arg->actLen = write(fd, buf, len);
    }
    if (arg->actLen == -1) {
        return UniError(errno);
    }

    return UniError(ERRNO_NOERR);
}

napi_value PropNExporter::Write(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    unique_ptr<char[]> bufGuard = nullptr;
    void *buf = nullptr;
    size_t len = 0;
    int64_t position = -1;
    tie(succ, bufGuard, buf, len, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    auto cbExec = [arg, buf, len, fd, position](napi_env env) -> UniError {
        return WriteExec(arg, buf, len, fd, position);
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, arg->actLen) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    }
}

napi_value PropNExporter::UnlinkSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    if (unlink(path.get()) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd = -1;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || fd < 0) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    size_t len = 0;
    int64_t position = -1;
    unique_ptr<char[]> bufGuard = nullptr;
    tie(succ, bufGuard, buf, len, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t writeLen = 0;
    if (position >= 0) {
        writeLen = pwrite(fd, buf, len, position);
    } else {
        writeLen = write(fd, buf, len);
    }

    if (writeLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

bool PropNExporter::ExportSync()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("accessSync", AccessSync),
        NVal::DeclareNapiFunction("chmodSync", Chmod::Sync),
        NVal::DeclareNapiFunction("chownSync", Chown::Sync),
        NVal::DeclareNapiFunction("closeSync", Close::Sync),
        NVal::DeclareNapiFunction("copyFileSync", CopyFile::Sync),
        NVal::DeclareNapiFunction("createStreamSync", CreateStream::Sync),
        NVal::DeclareNapiFunction("fchmodSync", Fchmod::Sync),
        NVal::DeclareNapiFunction("fchownSync", Fchown::Sync),
        NVal::DeclareNapiFunction("fdatasyncSync", Fdatasync::Sync),
        NVal::DeclareNapiFunction("fdopenStreamSync", FdopenStream::Sync),
        NVal::DeclareNapiFunction("fstatSync", Fstat::Sync),
        NVal::DeclareNapiFunction("fsyncSync", Fsync::Sync),
        NVal::DeclareNapiFunction("ftruncateSync", Ftruncate::Sync),
        NVal::DeclareNapiFunction("lchownSync", Lchown::Sync),
        NVal::DeclareNapiFunction("linkSync", Link::Sync),
        NVal::DeclareNapiFunction("lseekSync", Lseek::Sync),
        NVal::DeclareNapiFunction("lstatSync", Lstat::Sync),
        NVal::DeclareNapiFunction("mkdirSync", MkdirSync),
        NVal::DeclareNapiFunction("mkdtempSync", Mkdtemp::Sync),
        NVal::DeclareNapiFunction("openSync", Open::Sync),
        NVal::DeclareNapiFunction("opendirSync", OpenDir::Sync),
        NVal::DeclareNapiFunction("readdirSync", ReadDir::Sync),
        NVal::DeclareNapiFunction("posixFallocateSync", PosixFallocate::Sync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("readTextSync", ReadText::Sync),
        NVal::DeclareNapiFunction("renameSync", Rename::Sync),
        NVal::DeclareNapiFunction("rmdirSync", Rmdirent::Sync),
        NVal::DeclareNapiFunction("statSync", Stat::Sync),
        NVal::DeclareNapiFunction("symlinkSync", Symlink::Sync),
        NVal::DeclareNapiFunction("truncateSync", Truncate::Sync),
        NVal::DeclareNapiFunction("unlinkSync", UnlinkSync),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
    });
}

bool PropNExporter::ExportAsync()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("access", Access),
        NVal::DeclareNapiFunction("access", Access),
        NVal::DeclareNapiFunction("chmod", Chmod::Async),
        NVal::DeclareNapiFunction("chown", Chown::Async),
        NVal::DeclareNapiFunction("close", Close::Async),
        NVal::DeclareNapiFunction("copyFile", CopyFile::Async),
        NVal::DeclareNapiFunction("createStream", CreateStream::Async),
        NVal::DeclareNapiFunction("createWatcher", Watcher::CreateWatcher),
        NVal::DeclareNapiFunction("fchmod", Fchmod::Async),
        NVal::DeclareNapiFunction("fchown", Fchown::Async),
        NVal::DeclareNapiFunction("fdatasync", Fdatasync::Async),
        NVal::DeclareNapiFunction("fdopenStream", FdopenStream::Async),
        NVal::DeclareNapiFunction("fstat", Fstat::Async),
        NVal::DeclareNapiFunction("fsync", Fsync::Async),
        NVal::DeclareNapiFunction("ftruncate", Ftruncate::Async),
        NVal::DeclareNapiFunction("hash", Hash::Async),
        NVal::DeclareNapiFunction("lchown", Lchown::Async),
        NVal::DeclareNapiFunction("link", Link::Async),
        NVal::DeclareNapiFunction("lseek", Lseek::Async),
        NVal::DeclareNapiFunction("lstat", Lstat::Async),
        NVal::DeclareNapiFunction("mkdir", Mkdir),
        NVal::DeclareNapiFunction("mkdtemp", Mkdtemp::Async),
        NVal::DeclareNapiFunction("open", Open::Async),
        NVal::DeclareNapiFunction("opendir", OpenDir::Async),
        NVal::DeclareNapiFunction("readdir", ReadDir::Async),
        NVal::DeclareNapiFunction("posixFallocate", PosixFallocate::Async),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("readText", ReadText::Async),
        NVal::DeclareNapiFunction("rename", Rename::Async),
        NVal::DeclareNapiFunction("rmdir", Rmdirent::Async),
        NVal::DeclareNapiFunction("stat", Stat::Async),
        NVal::DeclareNapiFunction("symlink", Symlink::Async),
        NVal::DeclareNapiFunction("truncate", Truncate::Async),
        NVal::DeclareNapiFunction("unlink", Unlink),
        NVal::DeclareNapiFunction("write", Write),
    });
}

bool PropNExporter::Export()
{
    return ExportSync() && ExportAsync();
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
