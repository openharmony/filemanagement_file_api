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
#include "read_text.h"

#include <fcntl.h>
#include <securec.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>
#include "../../common/file_helper/fd_guard.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_func_arg.h"
#include "../common_func.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, ssize_t, bool, ssize_t, unique_ptr<char[]>, bool> GetReadTextArg(napi_env env, napi_value argOption)
{
    NVal op(env, argOption);
    ssize_t position = 0;
    ssize_t len = 0;
    bool succ = false;
    bool hasOp = false;
    bool hasLen = false;
    unique_ptr<char[]> encoding;

    if (op.HasProp("position")) {
        tie(succ, position) = op.GetProp("position").ToInt32();
        if (!succ) {
            return { false, position, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
    }

    if (op.HasProp("length")) {
        tie(succ, len) = op.GetProp("length").ToInt32();
        if (!succ) {
            return { false, position, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
        hasLen = true;
    }

    if (op.HasProp("encoding")) {
        auto [succ, encoding, unuse] = op.GetProp("encoding").ToUTF8String();
        if (!succ) {
            return { false, position, hasLen, len, nullptr, hasOp };
        }
        hasOp = true;
    }

    if (!op.TypeIs(napi_function)) {
        hasOp = true;
    }

    return { true, position, hasLen, len, move(encoding), hasOp };
}

napi_value ReadText::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto [resGetReadTextArg, position, hasLen, len, encoding, useless] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetReadTextArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid option");
        return nullptr;
    }

    struct stat statbf;
    int ret;
    FDGuard sfd;
    sfd.SetFD(open(path.get(), O_RDONLY));
    if ((!sfd) || (fstat(sfd.GetFD(), &statbf) == -1)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    if (position > statbf.st_size) {
        UniError(EINVAL).ThrowErr(env, "Invalid position");
        return nullptr;
    }

    len = !hasLen ? statbf.st_size : len;
    len = ((len  < statbf.st_size) ? len : statbf.st_size);
    std::unique_ptr<char[]> readbuf = std::make_unique<char[]>(len + 1);
    if (readbuf == nullptr) {
        UniError(EINVAL).ThrowErr(env, "file is too large");
        return nullptr;
    }

    if (memset_s(readbuf.get(), len + 1, 0, len + 1) != EOK) {
        UniError(errno).ThrowErr(env, "dfs mem error");
        return nullptr;
    }

    ret = position > 0 ? pread(sfd.GetFD(), readbuf.get(), len, position) : read(sfd.GetFD(), readbuf.get(), len);
    if (ret == -1) {
        UniError(EINVAL).ThrowErr(env, "Invalid read file");
        return nullptr;
    }

    return NVal::CreateUTF8String(env, readbuf.get(), ret).val_;
}

UniError ReadText::AsyncExec(const std::string &path, std::shared_ptr<AsyncReadTextArg> arg, ssize_t position,
    bool hasLen, ssize_t len)
{
    if (arg == nullptr) {
        return UniError(ENOMEM);
    }

    FDGuard sfd;
    struct stat statbf;
    arg->len = len;
    sfd.SetFD(open(path.c_str(), O_RDONLY));
    if (sfd.GetFD() == -1) {
        return UniError(EINVAL);
    }

    if (fstat(sfd.GetFD(), &statbf) == -1) {
        return UniError(EINVAL);
    }

    if (position > statbf.st_size) {
        return UniError(EINVAL);
    }

    if (!hasLen) {
        arg->len = statbf.st_size;
    }

    arg->len = ((arg->len < statbf.st_size) ? arg->len : statbf.st_size);
    arg->buf = std::make_unique<char[]>(arg->len);
    if (arg->buf == nullptr) {
        return UniError(ENOMEM);
    }

    if (position > 0) {
        arg->len = pread(sfd.GetFD(), arg->buf.get(), arg->len, position);
    } else {
        arg->len = read(sfd.GetFD(), arg->buf.get(), arg->len);
    }

    if (arg->len == -1) {
        return UniError(EINVAL);
    }

    return UniError(ERRNO_NOERR);
}

napi_value ReadText::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto [resGetSecondArg, position, hasLen, len, encoding, hasOp] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetSecondArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid option");
        return nullptr;
    }

    auto arg = make_shared<AsyncReadTextArg>(NVal(env, funcArg.GetThisVar()));
    if (arg == nullptr) {
        return nullptr;
    }

    auto cbExec =
        [path = string(path.get()), arg, position = position, hasLen = hasLen, len = len](napi_env env) -> UniError {
        return AsyncExec(path, arg, position, hasLen, len);
    };

    auto cbComplete = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUTF8String(env, arg->buf.get(), arg->len);
        }
    };

    size_t argc = funcArg.GetArgc();
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE || (argc == NARG_CNT::TWO && hasOp)) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOReadText", cbExec, cbComplete).val_;
    } else {
        int cbIdx = !hasOp ? NARG_POS::SECOND : NARG_POS::THIRD;
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOReadText", cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS