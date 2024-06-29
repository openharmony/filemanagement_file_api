/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "hashstream_n_exporter.h"

#include <iomanip>
#include <sstream>

#include "file_utils.h"
#include "hashstream_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static HASH_ALGORITHM_TYPE GetHashAlgorithm(const string &alg)
{
    return (algorithmMaps.find(alg) != algorithmMaps.end()) ? algorithmMaps.at(alg) : HASH_ALGORITHM_TYPE_UNSUPPORTED;
}

static tuple<bool, HashStreamEntity*> GetHsEntity(napi_env env, napi_value hs_entity)
{
    auto hsEntity = NClass::GetEntityOf<HashStreamEntity>(env, hs_entity);
    if (!hsEntity) {
        return { false, nullptr };
    }
    return { true, hsEntity };
}

static string HashFinal(const unique_ptr<unsigned char[]> &hashBuf, size_t hashLen)
{
    stringstream ss;
    for (size_t i = 0; i < hashLen; ++i) {
        const int hexPerByte = 2;
        ss << std::uppercase << std::setfill('0') << std::setw(hexPerByte) << std::hex
           << static_cast<uint32_t>(hashBuf[i]);
    }

    return ss.str();
}

static napi_value SetHsEntity(napi_env env, NFuncArg &funcArg, HASH_ALGORITHM_TYPE algType)
{
    auto hsEntity = CreateUniquePtr<HashStreamEntity>();
    if (hsEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    hsEntity->algType = algType;

    switch (algType) {
        case HASH_ALGORITHM_TYPE_MD5: {
            MD5_CTX ctx;
            MD5_Init(&ctx);
            hsEntity->md5Ctx = ctx;
            break;
        }
        case HASH_ALGORITHM_TYPE_SHA1: {
            SHA_CTX ctx;
            SHA1_Init(&ctx);
            hsEntity->shaCtx = ctx;
            break;
        }
        case HASH_ALGORITHM_TYPE_SHA256: {
            SHA256_CTX ctx;
            SHA256_Init(&ctx);
            hsEntity->sha256Ctx = ctx;
            break;
        }
        default:
            break;
    }
    if (!NClass::SetEntityFor<HashStreamEntity>(env, funcArg.GetThisVar(), move(hsEntity))) {
        HILOGE("INNER BUG. Failed to wrap entity for obj hsEntity");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

napi_value HashStreamNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succAlg, alg, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succAlg) {
        HILOGE("Invalid algorithm");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    HASH_ALGORITHM_TYPE algType = GetHashAlgorithm(alg.get());
    if (algType == HASH_ALGORITHM_TYPE_UNSUPPORTED) {
        HILOGE("Unsupport algorithm");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    return SetHsEntity(env, funcArg, algType);
}

napi_value HashStreamNExporter::Digest(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succEntity, hsEntity] = GetHsEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    string digestStr;
    switch (hsEntity->algType) {
        case HASH_ALGORITHM_TYPE_MD5: {
            auto res = make_unique<unsigned char[]>(MD5_DIGEST_LENGTH);
            MD5_Final(res.get(), &hsEntity->md5Ctx);
            digestStr = HashFinal(res, MD5_DIGEST_LENGTH);
            break;
        }
        case HASH_ALGORITHM_TYPE_SHA1: {
            auto res = make_unique<unsigned char[]>(SHA_DIGEST_LENGTH);
            SHA1_Final(res.get(), &hsEntity->shaCtx);
            digestStr = HashFinal(res, SHA_DIGEST_LENGTH);
            break;
        }
        case HASH_ALGORITHM_TYPE_SHA256: {
            auto res = make_unique<unsigned char[]>(SHA256_DIGEST_LENGTH);
            SHA256_Final(res.get(), &hsEntity->sha256Ctx);
            digestStr = HashFinal(res, SHA256_DIGEST_LENGTH);
            break;
        }
        default:
            break;
    }

    return NVal::CreateUTF8String(env, digestStr).val_;
}

napi_value HashStreamNExporter::Update(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, buf, bufLen] = NVal(env, funcArg[NARG_POS::FIRST]).ToArraybuffer();
    if (!succ) {
        HILOGE("illegal array buffer");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succEntity, hsEntity] = GetHsEntity(env, funcArg.GetThisVar());
    if (!succEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }

    switch (hsEntity->algType) {
        case HASH_ALGORITHM_TYPE_MD5:
            MD5_Update(&hsEntity->md5Ctx, buf, bufLen);
            break;
        case HASH_ALGORITHM_TYPE_SHA1:
            SHA1_Update(&hsEntity->shaCtx, buf, bufLen);
            break;
        case HASH_ALGORITHM_TYPE_SHA256:
            SHA256_Update(&hsEntity->sha256Ctx, buf, bufLen);
            break;
        default:
            break;
    }
    return NVal::CreateUndefined(env).val_;
}

bool HashStreamNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("digest", Digest),
        NVal::DeclareNapiFunction("update", Update),
    };
    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        HashStreamNExporter::Constructor, move(props));
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("INNER BUG. Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string HashStreamNExporter::GetClassName()
{
    return HashStreamNExporter::className_;
}

HashStreamNExporter::HashStreamNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

HashStreamNExporter::~HashStreamNExporter() {}
}
}
}