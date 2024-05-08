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

static tuple<bool, int64_t, bool, size_t, unique_ptr<char[]>> GetReadTextArg(napi_env env, napi_value argOption)
{
    NVal op(env, argOption);
    int64_t position = -1;
    size_t len = 0;
    bool hasLen = false;
    unique_ptr<char[]> encoding = nullptr;

    if (op.HasProp("position") && !op.GetProp("position").TypeIs(napi_undefined)) {
        bool succ = false;
        tie(succ, position) = op.GetProp("position").ToInt64();
        if (!succ || position < 0) {
            return { false, position, hasLen, len, nullptr };
        }
    }

    if (op.HasProp("length") && !op.GetProp("length").TypeIs(napi_undefined)) {
        auto [succ, length] = op.GetProp("length").ToInt64();
        if (!succ || length < 0 || length > UINT_MAX) {
            return { false, position, hasLen, len, nullptr };
        }
        len = static_cast<size_t>(length);
        hasLen = true;
    }

    if (op.HasProp("encoding")) {
        auto [succ, encoding, unuse] = op.GetProp("encoding").ToUTF8String("utf-8");
        string_view encodingStr(encoding.get());
        if (!succ || encodingStr != "utf-8") {
            return { false, position, hasLen, len, nullptr };
        }
    }

    return { true, position, hasLen, len, move(encoding) };
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

    auto [resGetReadTextArg, position, hasLen, len, encoding] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
    if (!resGetReadTextArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid option");
        return nullptr;
    }

    struct stat statbf;
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

    len = (!hasLen || len > static_cast<size_t>(statbf.st_size)) ? statbf.st_size : len;
    std::unique_ptr<char[]> readbuf = std::make_unique<char[]>(len + 1);
    if (readbuf == nullptr) {
        UniError(EINVAL).ThrowErr(env, "file is too large");
        return nullptr;
    }

    if (memset_s(readbuf.get(), len + 1, 0, len + 1) != EOK) {
        UniError(errno).ThrowErr(env, "dfs mem error");
        return nullptr;
    }
    ssize_t ret = 0;
    if (position >= 0) {
        ret = pread(sfd.GetFD(), readbuf.get(), len, position);
    } else {
        ret = read(sfd.GetFD(), readbuf.get(), len);
    }
    if (ret == -1) {
        UniError(EINVAL).ThrowErr(env, "Invalid read file");
        return nullptr;
    }

    return NVal::CreateUTF8String(env, readbuf.get(), ret).val_;
}

static UniError AsyncExec(const std::string &path, std::shared_ptr<AsyncReadTextArg> arg, int64_t position,
    bool hasLen, size_t len)
{
    if (arg == nullptr) {
        return UniError(ENOMEM);
    }

    FDGuard sfd;
    struct stat statbf;
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

    len = (!hasLen || len > static_cast<size_t>(statbf.st_size)) ? statbf.st_size : len;
    arg->buf = std::make_unique<char[]>(len);
    if (arg->buf == nullptr) {
        return UniError(ENOMEM);
    }

    if (position >= 0) {
        arg->len = pread(sfd.GetFD(), arg->buf.get(), len, position);
    } else {
        arg->len = read(sfd.GetFD(), arg->buf.get(), len);
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

    auto [resGetSecondArg, position, hasLen, len, encoding] = GetReadTextArg(env, funcArg[NARG_POS::SECOND]);
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

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule("FileIOReadText", cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule("FileIOReadText", cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS