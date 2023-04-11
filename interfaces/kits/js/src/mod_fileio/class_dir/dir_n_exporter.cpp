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

#include "dir_n_exporter.h"

#include <dirent.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

#include "dir_entity.h"
#include "securec.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../class_dirent/dirent_entity.h"
#include "../class_dirent/dirent_n_exporter.h"
#include "../common_func.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static DirEntity *GetDirEntity(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, funcArg.GetThisVar());
    if (!dirEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dir");
        return nullptr;
    }
    return dirEntity;
}

napi_value DirNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    DirEntity *dirEntity = GetDirEntity(env, info);
    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }

    lock_guard(dirEntity->lock_);
    dirEntity->dir_.reset();
    return nullptr;
}

napi_value DirNExporter::Close(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, funcArg.GetThisVar());
    if (!dirEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dir");
        return nullptr;
    }

    if (dirEntity->dir_ == nullptr) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }

    auto cbExec = [dirEntity](napi_env env) -> UniError {
        lock_guard(dirEntity->lock_);
        DIR *dir = dirEntity->dir_.release();
        int ret = closedir(dir);
        if (ret == -1) {
            return UniError(errno);
        } else {
            return UniError(ERRNO_NOERR);
        }
    };
    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    static const string PROCEDURE_NAME = "fileioDirClose";
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbCompl).val_;
    }
}

struct DirReadArgs {
    NRef thisptrRef_;
    struct dirent dirRes = {
        .d_ino = 0,
        .d_off = 0,
        .d_reclen = 0,
        .d_type = 0,
        .d_name = { '\0' },
    };
    explicit DirReadArgs(NVal obj) : thisptrRef_(obj) {}
};

static NVal DoReadComplete(napi_env env, UniError err, shared_ptr<DirReadArgs> arg)
{
    if (err) {
        return { env, err.GetNapiErr(env) };
    } else {
        napi_value objDirent = NClass::InstantiateClass(env, DirentNExporter::className_, {});
        if (!objDirent) {
            return { env, UniError(EINVAL).GetNapiErr(env) };
        }
        auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, objDirent);
        if (!direntEntity) {
            return { env, UniError(EINVAL).GetNapiErr(env) };
        }

        if (strlen(arg->dirRes.d_name) == 0) {
            return { env, NVal::CreateUndefined(env).val_ };
        } else {
            direntEntity->dirent_ = arg->dirRes;
            return { env, objDirent };
        }
    }
}

napi_value DirNExporter::Read(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, funcArg.GetThisVar());
    if (!dirEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dir");
        return nullptr;
    }

    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }

    DIR *dir = dirEntity->dir_.get();
    auto arg = make_shared<DirReadArgs>(NVal(env, funcArg.GetThisVar()));
    auto cbExec = [arg, dir, dirEntity](napi_env env) -> UniError {
        struct dirent tmpDirent;
        lock_guard(dirEntity->lock_);
        errno = 0;
        dirent *res = nullptr;
        do {
            res = readdir(dir);
            if (res == nullptr && errno) {
                return UniError(errno);
            } else if (res == nullptr) {
                return UniError(ERRNO_NOERR);
            } else if (string(res->d_name) == "." || string(res->d_name) == "..") {
                continue;
            } else {
                if (EOK != memcpy_s(&tmpDirent, sizeof(dirent), res, res->d_reclen)) {
                    return UniError(errno);
                }
                break;
            }
        } while (true);

        arg->dirRes = tmpDirent;
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        return DoReadComplete(env, err, arg);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    const string PROCEDURE_NAME = "fileioDirRead";
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbCompl).val_;
    }
}

napi_value DirNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    
    DirEntity *dirEntity = GetDirEntity(env, info);
    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }
    DIR *dir = dirEntity->dir_.get();

    struct dirent tmpDirent;
    {
        lock_guard(dirEntity->lock_);
        errno = 0;
        dirent *res = nullptr;
        do {
            res = readdir(dir);
            if (res == nullptr && errno) {
                UniError(errno).ThrowErr(env);
                return nullptr;
            } else if (res == nullptr) {
                return NVal::CreateUndefined(env).val_;
            } else if (string(res->d_name) == "." || string(res->d_name) == "..") {
                continue;
            } else {
                if (EOK != memcpy_s(&tmpDirent, sizeof(dirent), res, res->d_reclen)) {
                    UniError(errno).ThrowErr(env);
                    return nullptr;
                }
                break;
            }
        } while (true);
    }

    napi_value objDirent = NClass::InstantiateClass(env, DirentNExporter::className_, {});
    if (!objDirent) {
        return nullptr;
    }

    auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, objDirent);
    if (!direntEntity) {
        return nullptr;
    }
    direntEntity->dirent_ = tmpDirent;

    return objDirent;
}

struct DirListFileArgs {
    vector<dirent> dirents;
    explicit DirListFileArgs()
    {
        dirents = vector<dirent>();
    }
    ~DirListFileArgs() = default;
};

static DirEntity *CheckDirEntity(napi_env env, napi_value dir_entity)
{
    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, dir_entity);
    if (!dirEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dir");
        return nullptr;
    }

    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }
    return dirEntity;
}

static tuple<bool, int> ParseJsListNum(napi_env env, napi_value listNumFromJs)
{
    auto [succ, listNum] = NVal(env, listNumFromJs).ToInt32();
    return {succ, listNum};
}

static napi_value DoListFileVector2NV(napi_env env, vector<dirent> dirents)
{
    napi_value res = nullptr;
    napi_create_array(env, &res);
    for (size_t i = 0; i < dirents.size(); i++) {
        napi_value objDirent = NClass::InstantiateClass(env, DirentNExporter::className_, {});
        if (!objDirent) {
            UniError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, objDirent);
        if (!direntEntity) {
            UniError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        direntEntity->dirent_ = dirents[i];
        napi_set_element(env, res, i, objDirent);
    }
    return res;
}

static NVal DoListFileCompile(napi_env env, UniError err, shared_ptr<DirListFileArgs> arg)
{
    if (err) {
        return { env, err.GetNapiErr(env) };
    } else {
        return { env, DoListFileVector2NV(env, arg->dirents) };
    }
}

napi_value DirNExporter::ListFile(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto dirEntity = CheckDirEntity(env, funcArg.GetThisVar());
    if (!dirEntity) {
        return nullptr;
    }
    auto [succ, num] = ParseJsListNum(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid listNum");
        return nullptr;
    }

    DIR *dir = dirEntity->dir_.get();
    auto arg = make_shared<DirListFileArgs>();
    int listNum = num;
    auto cbExec = [arg, dir, dirEntity, listNum](napi_env env) -> UniError {
        lock_guard(dirEntity->lock_);
        errno = 0;
        dirent *res = nullptr;
        int listCount = 0;
        do {
            res = readdir(dir);
            if (res == nullptr && errno) {
                return UniError(errno);
            } else if (res == nullptr) {
                return UniError(ERRNO_NOERR);
            } else if (string(res->d_name) == "." || string(res->d_name) == "..") {
                continue;
            } else {
                arg->dirents.push_back(*res);
                listCount++;
            }
        } while (listCount < listNum || listNum == 0);
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        return DoListFileCompile(env, err, arg);
    };
    NVal thisVar(env, funcArg.GetThisVar());

    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(listfileProcedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(listfileProcedureName, cbExec, cbCompl).val_;
    }
}

napi_value DirNExporter::ListFileSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto dirEntity = CheckDirEntity(env, funcArg.GetThisVar());
    if (!dirEntity) {
        return nullptr;
    }
    auto [succ, listNum] = ParseJsListNum(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid listNum");
        return nullptr;
    }

    vector<dirent> dirents;
    {
        lock_guard(dirEntity->lock_);
        errno = 0;
        dirent *res = nullptr;
        int listCount = 0;
        auto dir = dirEntity->dir_.get();
        do {
            res = readdir(dir);
            if (res == nullptr && errno) {
                UniError(errno).ThrowErr(env);
                return nullptr;
            } else if (res == nullptr) {
                break;
            } else if (string(res->d_name) == "." || string(res->d_name) == "..") {
                continue;
            } else {
                dirents.push_back(*res);
                listCount++;
            }
        } while (listCount < listNum || listNum == 0);
    }
    return DoListFileVector2NV(env, dirents);
}

napi_value DirNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = make_unique<DirEntity>();
    if (!NClass::SetEntityFor<DirEntity>(env, funcArg.GetThisVar(), move(dirEntity))) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj dir");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool DirNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
        NVal::DeclareNapiFunction("listfileSync", ListFileSync),
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("close", Close),
        NVal::DeclareNapiFunction("listfile", ListFile),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, DirNExporter::Constructor, std::move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class Dirent");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class Dirent");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string DirNExporter::GetClassName()
{
    return DirNExporter::className_;
}

DirNExporter::DirNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

DirNExporter::~DirNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
