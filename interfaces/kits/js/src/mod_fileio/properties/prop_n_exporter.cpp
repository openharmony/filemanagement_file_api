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
#include "create_randomaccessfile.h"
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
    static constexpr int MODE_RUO_RWX = 0750;
}
napi_value PropNExporter::AccessSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
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

    size_t argc = funcArg.GetArgc();
    int ret = -1;
    if (argc == NARG_CNT::ONE) {
        ret = access(path.get(), 0);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        ret = access(path.get(), mode);
    }

    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

static tuple<bool, string, int, bool> GetAccessArgs(napi_env env, const NFuncArg &funcArg)
{
    bool succ = false;
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return { false, nullptr, 0, false };
    }

    size_t argc = funcArg.GetArgc();
    bool promise = true;
    bool hasMode = false;
    if (argc == NARG_CNT::ONE) {
        hasMode = false;
        promise = true;
    } else if (argc == NARG_CNT::TWO) {
        if (NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
            hasMode = false;
            promise = false;
        } else {
            hasMode = true;
            promise = true;
        }
    } else {
        hasMode = true;
        promise = false;
    }

    int mode = 0;
    if (hasMode) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return { false, nullptr, 0, false };
        }
    }

    return { true, path.get(), mode, promise };
}

struct AsyncAccessArg {
    unique_ptr<char[]> fp = nullptr;
};

napi_value PropNExporter::Access(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    string path;
    bool succ = false;
    bool promise = false;
    int mode;
    tie(succ, path, mode, promise) = GetAccessArgs(env, funcArg);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto cbExec = [path = move(path), mode](napi_env env) -> UniError {
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
    if (promise) {
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

    string path;
    unique_ptr<char[]> tmp;
    bool succ = false;
    tie(succ, tmp, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "invalid path");
        return nullptr;
    }
    path = tmp.get();

    auto cbExec = [path](napi_env env) -> UniError {
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

    string path;
    unique_ptr<char[]> tmp;
    bool succ = false;
    tie(succ, tmp, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }
    path = tmp.get();
    int mode = 0775;
    size_t argc = funcArg.GetArgc();
    if ((argc == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) ||
        argc == NARG_CNT::THREE) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
    }
    auto cbExec = [path, mode](napi_env env) -> UniError {
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
    if ((argc == NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) || argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((argc == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD);
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

    bool succ = false;
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    int ret = -1;
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::ONE) {
        ret = mkdir(path.get(), MODE_RUO_RWX);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        ret = mkdir(path.get(), mode);
    }

    if (ret == -1) {
        HILOGE("errno = %{public}d", errno);
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
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int mode;
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

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
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

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    int64_t len;
    bool hasPos = false;
    int64_t pos;
    tie(succ, buf, len, hasPos, pos, ignore) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t actLen;
    if (hasPos) {
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
    int offset { 0 };
    NRef refReadBuf;

    explicit AsyncIOReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncIOReadArg() = default;
};

napi_value PropNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    void *buf = nullptr;
    int64_t len;
    int fd;
    bool hasPos = false;
    int64_t pos;
    int offset;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    tie(succ, buf, len, hasPos, pos, offset) =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Failed GetReadArg");
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    auto cbExec = [arg, buf, len, fd, hasPos, pos, offset](napi_env env) -> UniError {
        ssize_t actLen;
        if (hasPos) {
            actLen = pread(fd, buf, len, pos);
        } else {
            actLen = read(fd, buf, len);
        }
        if (actLen == -1) {
            return UniError(errno);
        } else {
            arg->lenRead = actLen;
            arg->offset = offset;
            return UniError(ERRNO_NOERR);
        }
    };

    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
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
    };

    NVal thisVar(env, funcArg.GetThisVar());
    size_t argc = funcArg.GetArgc();
    bool hasOp = false;
    if (argc == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length")|| !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }

    if (argc == NARG_CNT::TWO || (argc == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIORead", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((argc == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIORead", cbExec, cbCompl).val_;
    }
}

UniError PropNExporter::WriteExec(shared_ptr<AsyncIOWrtieArg> arg, void *buf, size_t len, int fd, size_t position)
{
    if (position == (size_t)INVALID_POSITION) {
        arg->actLen = write(fd, buf, len);
    } else {
        arg->actLen = pwrite(fd, buf, len, position);
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
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    unique_ptr<char[]> bufGuard;
    void *buf = nullptr;
    size_t len;
    size_t position;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Failed GetWriteArg");
        return nullptr;
    }

    shared_ptr<AsyncIOWrtieArg> arg;
    if (bufGuard) {
        arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    } else {
        arg = make_shared<AsyncIOWrtieArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    }
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
    bool hasOp = false;
    size_t argc = funcArg.GetArgc();
    if (argc == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length") ||
            op.HasProp("encoding") || !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }

    if (argc == NARG_CNT::TWO || (argc == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((argc == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
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
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    size_t len;
    size_t position;
    unique_ptr<char[]> bufGuard;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t writeLen;
    if (position == (size_t)INVALID_POSITION) {
        writeLen = write(fd, buf, len);
    } else {
        writeLen = pwrite(fd, buf, len, position);
    }

    if (writeLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

bool PropNExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("access", Access),
        NVal::DeclareNapiFunction("accessSync", AccessSync),
        NVal::DeclareNapiFunction("chmod", Chmod::Async),
        NVal::DeclareNapiFunction("chmodSync", Chmod::Sync),
        NVal::DeclareNapiFunction("chown", Chown::Async),
        NVal::DeclareNapiFunction("chownSync", Chown::Sync),
        NVal::DeclareNapiFunction("close", Close::Async),
        NVal::DeclareNapiFunction("closeSync", Close::Sync),
        NVal::DeclareNapiFunction("copyFile", CopyFile::Async),
        NVal::DeclareNapiFunction("copyFileSync", CopyFile::Sync),
        NVal::DeclareNapiFunction("createRandomAccessFile", CreateRandomAccessFile::Async),
        NVal::DeclareNapiFunction("createRandomAccessFileSync", CreateRandomAccessFile::Sync),
        NVal::DeclareNapiFunction("createStream", CreateStream::Async),
        NVal::DeclareNapiFunction("createStreamSync", CreateStream::Sync),
        NVal::DeclareNapiFunction("createWatcher", Watcher::CreateWatcher),
        NVal::DeclareNapiFunction("fchmod", Fchmod::Async),
        NVal::DeclareNapiFunction("fchmodSync", Fchmod::Sync),
        NVal::DeclareNapiFunction("fchown", Fchown::Async),
        NVal::DeclareNapiFunction("fchownSync", Fchown::Sync),
        NVal::DeclareNapiFunction("fdatasync", Fdatasync::Async),
        NVal::DeclareNapiFunction("fdatasyncSync", Fdatasync::Sync),
        NVal::DeclareNapiFunction("fdopenStream", FdopenStream::Async),
        NVal::DeclareNapiFunction("fdopenStreamSync", FdopenStream::Sync),
        NVal::DeclareNapiFunction("fstat", Fstat::Async),
        NVal::DeclareNapiFunction("fstatSync", Fstat::Sync),
        NVal::DeclareNapiFunction("fsync", Fsync::Async),
        NVal::DeclareNapiFunction("fsyncSync", Fsync::Sync),
        NVal::DeclareNapiFunction("ftruncate", Ftruncate::Async),
        NVal::DeclareNapiFunction("ftruncateSync", Ftruncate::Sync),
        NVal::DeclareNapiFunction("hash", Hash::Async),
        NVal::DeclareNapiFunction("lchown", Lchown::Async),
        NVal::DeclareNapiFunction("lchownSync", Lchown::Sync),
        NVal::DeclareNapiFunction("link", Link::Async),
        NVal::DeclareNapiFunction("linkSync", Link::Sync),
        NVal::DeclareNapiFunction("lseek", Lseek::Async),
        NVal::DeclareNapiFunction("lseekSync", Lseek::Sync),
        NVal::DeclareNapiFunction("lstat", Lstat::Async),
        NVal::DeclareNapiFunction("lstatSync", Lstat::Sync),
        NVal::DeclareNapiFunction("mkdir", Mkdir),
        NVal::DeclareNapiFunction("mkdirSync", MkdirSync),
        NVal::DeclareNapiFunction("mkdtemp", Mkdtemp::Async),
        NVal::DeclareNapiFunction("mkdtempSync", Mkdtemp::Sync),
        NVal::DeclareNapiFunction("open", Open::Async),
        NVal::DeclareNapiFunction("openSync", Open::Sync),
        NVal::DeclareNapiFunction("opendir", OpenDir::Async),
        NVal::DeclareNapiFunction("opendirSync", OpenDir::Sync),
        NVal::DeclareNapiFunction("readdir", ReadDir::Async),
        NVal::DeclareNapiFunction("readdirSync", ReadDir::Sync),
        NVal::DeclareNapiFunction("posixFallocate", PosixFallocate::Async),
        NVal::DeclareNapiFunction("posixFallocateSync", PosixFallocate::Sync),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("readText", ReadText::Async),
        NVal::DeclareNapiFunction("readTextSync", ReadText::Sync),
        NVal::DeclareNapiFunction("rename", Rename::Async),
        NVal::DeclareNapiFunction("renameSync", Rename::Sync),
        NVal::DeclareNapiFunction("rmdir", Rmdirent::Async),
        NVal::DeclareNapiFunction("rmdirSync", Rmdirent::Sync),
        NVal::DeclareNapiFunction("stat", Stat::Async),
        NVal::DeclareNapiFunction("statSync", Stat::Sync),
        NVal::DeclareNapiFunction("symlink", Symlink::Async),
        NVal::DeclareNapiFunction("symlinkSync", Symlink::Sync),
        NVal::DeclareNapiFunction("truncate", Truncate::Async),
        NVal::DeclareNapiFunction("truncateSync", Truncate::Sync),
        NVal::DeclareNapiFunction("unlink", Unlink),
        NVal::DeclareNapiFunction("unlinkSync", UnlinkSync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
    });
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
