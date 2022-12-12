/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <uv.h>

#include "../common_func.h"

#include "filemgmt_libn.h"
#include "../class_file/file_entity.h"
#include "../class_file/file_n_exporter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value PropNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [res, buf, len, hasPos, pos] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!res) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    ssize_t actLen;
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), len);
    uv_fs_t read_req;
    int ret = uv_fs_read(nullptr, &read_req, fd, &buffer, 1, pos, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    actLen = read_req.result;
    uv_fs_req_cleanup(&read_req);

    return NVal::CreateInt64(env, actLen).val_;
}

struct AsyncIOReadArg {
    ssize_t lenRead { 0 };
};

static NError ReadExec(shared_ptr<AsyncIOReadArg> arg, void *buf, size_t len, int fd, size_t position)
{
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), len);
    uv_fs_t read_req;
    int ret = uv_fs_read(nullptr, &read_req, fd, &buffer, 1, static_cast<int>(position), nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        return NError(errno);
    }
    arg->lenRead = read_req.result;
    uv_fs_req_cleanup(&read_req);
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

    auto [succ, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [res, buf, len, hasPos, pos] =
        CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!res) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncIOReadArg>();
    auto cbExec = [arg, buf = buf, len = len, fd = fd, pos = pos, env = env]() -> NError {
        return ReadExec(arg, buf, len, fd, pos);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, arg->lenRead) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") ||  op.HasProp("length")|| !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIORead", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIORead", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

NError PropNExporter::WriteExec(shared_ptr<AsyncIOWrtieArg> arg, void *buf, size_t len, int fd, size_t position)
{
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), len);
    uv_fs_t write_req;
    int ret = uv_fs_write(nullptr, &write_req, fd, &buffer, 1, static_cast<int>(position), nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        return NError(errno);
    }
    arg->actLen = write_req.result;
    uv_fs_req_cleanup(&write_req);
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

    auto [succ, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [res, bufGuard, buf, len, hasPos, position] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!res) {
        HILOGE("Failed to resolve buf and options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    shared_ptr<AsyncIOWrtieArg> arg;
    if (bufGuard) {
        arg = make_shared<AsyncIOWrtieArg>(move(bufGuard));
    } else {
        arg = make_shared<AsyncIOWrtieArg>(NVal(env, funcArg[NARG_POS::SECOND]));
    }
    auto cbExec = [arg, buf = buf, len = len, fd = fd, position = position, env = env]() -> NError {
        return WriteExec(arg, buf, len, fd, position);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return { NVal::CreateInt64(env, arg->actLen) };
        }
    };

    NVal thisVar(env, funcArg.GetThisVar());
    bool hasOp = false;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        NVal op = NVal(env, funcArg[NARG_POS::THIRD]);
        if (op.HasProp("offset") || op.HasProp("position") || op.HasProp("length") ||
            op.HasProp("encoding") || !op.TypeIs(napi_function)) {
            hasOp = true;
        }
    }

    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOWrite", cbExec, cbCompl).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [res, bufGuard, buf, len, hasPos, position] =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!res) {
        HILOGE("Failed to resolve buf and options");
        return nullptr;
    }

    ssize_t writeLen;
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), len);
    uv_fs_t write_req;
    int ret = uv_fs_write(nullptr, &write_req, fd, &buffer, 1, static_cast<int>(position), nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    writeLen = write_req.result;
    uv_fs_req_cleanup(&write_req);

    return NVal::CreateInt64(env, writeLen).val_;
}

bool PropNExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("read", Read),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("write", Write),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
    });
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporter::~PropNExporter() {}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS