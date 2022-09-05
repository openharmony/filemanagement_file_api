/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hash.h"

#include <cstring>
#include <string_view>
#include <tuple>

#include "../../common/file_helper/hash_file.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

enum HASH_ALGORITHM_TYPE {
    HASH_ALGORITHM_TYPE_MD5,
    HASH_ALGORITHM_TYPE_SHA1,
    HASH_ALGORITHM_TYPE_SHA256,
    HASH_ALGORITHM_TYPE_UNSUPPORTED,
};

static HASH_ALGORITHM_TYPE GetHashAlgorithm(const unique_ptr<char[]> &alg, const size_t algLen)
{
    if (algLen == string_view("md5").length() && string_view(alg.get()).compare("md5") == 0) {
        return HASH_ALGORITHM_TYPE_MD5;
    } else if (algLen == string_view("sha1").length() && string_view(alg.get()).compare("sha1") == 0) {
        return HASH_ALGORITHM_TYPE_SHA1;
    } else if (algLen == string_view("sha256").length() && string_view(alg.get()).compare("sha256") == 0) {
        return HASH_ALGORITHM_TYPE_SHA256;
    } else {
        return HASH_ALGORITHM_TYPE_UNSUPPORTED;
    }
}

static tuple<bool, unique_ptr<char[]>, HASH_ALGORITHM_TYPE, bool> GetHashArgs(napi_env env, const NFuncArg &funcArg)
{
    bool isPromise = false;
    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    auto [resGetSecondArg, alg, algLen] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        UniError(EINVAL).ThrowErr(env, "Invalid algorithm");
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    HASH_ALGORITHM_TYPE algType = GetHashAlgorithm(alg, algLen);
    if (algType == HASH_ALGORITHM_TYPE_UNSUPPORTED) {
        UniError(EINVAL).ThrowErr(env, "Invalid algorithm");
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    if (funcArg.GetArgc() == NARG_CNT::THREE && !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        UniError(EINVAL).ThrowErr(env, "Invalid callback");
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    isPromise = funcArg.GetArgc() == NARG_CNT::TWO;
    return { true, move(path), algType, isPromise };
}

napi_value Hash::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, fpath, algType, isPromise] = GetHashArgs(env, funcArg);
    if (!succ) {
        return nullptr;
    }

    auto arg = make_shared<string>();
    auto cbExec = [fpath = string(fpath.release()), arg, algType = algType](napi_env env) -> UniError {
        int ret = EIO;
        string &res = *arg;
        if (algType == HASH_ALGORITHM_TYPE_MD5) {
            tie(ret, res) = HashFile::HashWithMD5(fpath);
        } else if (algType == HASH_ALGORITHM_TYPE_SHA1) {
            tie(ret, res) = HashFile::HashWithSHA1(fpath);
        } else if (algType == HASH_ALGORITHM_TYPE_SHA256) {
            tie(ret, res) = HashFile::HashWithSHA256(fpath);
        }
        return UniError(ret);
    };

    auto cbComplete = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { NVal(env, err.GetNapiErr(env)) };
        }

        return { NVal::CreateUTF8String(env, *arg) };
    };

    const string procedureName = "FileIOHash";
    NVal thisVar(env, funcArg.GetThisVar());
    if (isPromise) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS