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
#include "read_text.h"

#include <fcntl.h>
#include <securec.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, ssize_t, bool, ssize_t, unique_ptr<char[]>, bool> GetReadTextArg(napi_env env, napi_value argOption)
{
    NVal op(env, argOption);
    ssize_t offset = 0;
    ssize_t len = 0;
    bool succ = false;
    bool hasOp = false;
    bool hasLen = false;
    unique_ptr<char[]> encoding;

    if (op.HasProp("offset")) {
        tie(succ, offset) = op.GetProp("offset").ToInt32();
        if (!succ || offset < 0) {
            HILOGE("Illegal option.offset parameter");
            return { false, offset, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
    }

    if (op.HasProp("length")) {
        tie(succ, len) = op.GetProp("length").ToInt32();
        if (!succ || len < 0) {
            HILOGE("Illegal option.length parameter");
            return { false, offset, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
        hasLen = true;
    }

    if (op.HasProp("encoding")) {
        tie(succ, encoding, ignore) = op.GetProp("encoding").ToUTF8String();
        if (!succ) {
            HILOGE("Illegal option.encoding parameter");
            return { false, offset, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
    }

    if (!op.TypeIs(napi_function)) {
        hasOp = true;
    }

    return { true, offset, hasLen, len, move(encoding), hasOp };
}

static NError ReadTextAsync(const std::string &path, std::shared_ptr<AsyncReadTextArg> arg, ssize_t offset,
                            bool hasLen, ssize_t len)
{
    OHOS::DistributedFS::FDGuard sfd;
    struct stat statbf;
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), O_RDONLY,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open file, ret: %{public}d", ret);
        return NError(errno);
    }

    sfd.SetFD(ret);
    if (sfd.GetFD() < 0) {
        HILOGE("Failed to open file by path");
        return NError(errno);
    }
    if (fstat(sfd.GetFD(), &statbf) < 0) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        return NError(errno);
    }

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset");
        return NError(EINVAL);
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), len);
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    arg->len = uv_fs_read(nullptr, read_req.get(), sfd.GetFD(), &readbuf, 1, offset, nullptr);
    if (arg->len < 0) {
        HILOGE("Failed to read file by fd: %{public}d", sfd.GetFD());
        return NError(errno);
    }
    arg->buffer = buffer;
    return NError(ERRNO_NOERR);
}

napi_value ReadText::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetReadTextArg, offset, hasLen, len, encoding, useless] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetReadTextArg) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    OHOS::DistributedFS::FDGuard sfd;
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.get(), O_RDONLY,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open file by ret: %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    sfd.SetFD(ret);
    struct stat statbf;
    if ((!sfd) || (fstat(sfd.GetFD(), &statbf) < 0)) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset: %{public}zd", offset);
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), len);
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> read_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!read_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    ret = uv_fs_read(nullptr, read_req.get(), sfd.GetFD(), &readbuf, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file by fd: %{public}d", sfd.GetFD());
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUTF8String(env, readbuf.base, ret).val_;
}

napi_value ReadText::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetSecondArg, offset, hasLen, len, encoding, hasOp] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetSecondArg) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<AsyncReadTextArg>(NVal(env, funcArg.GetThisVar()));
    auto cbExec = [path = string(path.get()), arg, offset = offset, hasLen = hasLen, len = len]() -> NError {
        return ReadTextAsync(path, arg, offset, hasLen, len);
    };

    auto cbComplete = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUTF8String(env, arg->buffer.c_str(), arg->len);
        }
    };

    size_t argc = funcArg.GetArgc();
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE || (argc == NARG_CNT::TWO && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READTEXT_NAME, cbExec, cbComplete).val_;
    } else {
        int cbIdx = !hasOp ? NARG_POS::SECOND : NARG_POS::THIRD;
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READTEXT_NAME, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS