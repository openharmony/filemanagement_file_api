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

#include "hash.h"

#include <cstring>
#include <string_view>
#include <tuple>

#include "filemgmt_libhilog.h"
#include "hash_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static HASH_ALGORITHM_TYPE GetHashAlgorithm(const string &alg)
{
    return (algorithmMaps.find(alg) != algorithmMaps.end()) ? algorithmMaps.at(alg) : HASH_ALGORITHM_TYPE_UNSUPPORTED;
}

static tuple<bool, unique_ptr<char[]>, HASH_ALGORITHM_TYPE, bool> GetHashArgs(napi_env env, const NFuncArg &funcArg)
{
    bool isPromise = false;
    auto [resGetFirstArg, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    auto [resGetSecondArg, alg, ignore] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!resGetSecondArg) {
        HILOGE("Invalid algorithm");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    HASH_ALGORITHM_TYPE algType = GetHashAlgorithm(alg.get());
    if (algType == HASH_ALGORITHM_TYPE_UNSUPPORTED) {
        HILOGE("Invalid algorithm");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    if (funcArg.GetArgc() == NARG_CNT::THREE && !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function)) {
        HILOGE("Invalid callback");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, HASH_ALGORITHM_TYPE_UNSUPPORTED, isPromise };
    }

    isPromise = funcArg.GetArgc() == NARG_CNT::TWO;
    return { true, move(path), algType, isPromise };
}

napi_value Hash::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, fpath, algType, isPromise] = GetHashArgs(env, funcArg);
    if (!succ) {
        HILOGE("Failed to get hash args");
        return nullptr;
    }

    auto arg = make_shared<string>();
    auto cbExec = [fpath = string(fpath.release()), arg, algType = algType, env = env]() -> NError {
        int ret = EIO;
        string &res = *arg;
        if (algType == HASH_ALGORITHM_TYPE_MD5) {
            tie(ret, res) = DistributedFS::HashFile::HashWithMD5(fpath);
        } else if (algType == HASH_ALGORITHM_TYPE_SHA1) {
            tie(ret, res) = DistributedFS::HashFile::HashWithSHA1(fpath);
        } else if (algType == HASH_ALGORITHM_TYPE_SHA256) {
            tie(ret, res) = DistributedFS::HashFile::HashWithSHA256(fpath);
        }
        return NError(ret);
    };

    auto cbComplete = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { NVal(env, err.GetNapiErr(env)) };
        }

        return { NVal::CreateUTF8String(env, *arg) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (isPromise) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_HASH_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_HASH_NAME, cbExec, cbComplete).val_;
    }
}

bool HashNExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("hash", Hash::Async),
    });
}

string HashNExporter::GetClassName()
{
    return HashNExporter::className_;
}

HashNExporter::HashNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS