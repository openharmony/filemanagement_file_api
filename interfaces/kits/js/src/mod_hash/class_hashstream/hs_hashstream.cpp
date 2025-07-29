/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hs_hashstream.h"

#include <iomanip>
#include <sstream>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static HASH_ALGORITHM_TYPE GetHashAlgorithm(const string &alg)
{
    return (algorithmMaps.find(alg) != algorithmMaps.end()) ? algorithmMaps.at(alg) : HASH_ALGORITHM_TYPE_UNSUPPORTED;
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

tuple<bool, HsHashStreamEntity *> HsHashStream::GetHsEntity()
{
    if (!entity) {
        return { false, nullptr };
    }

    return { true, entity.get() };
}

FsResult<void> HsHashStream::Update(ArrayBuffer &buffer)
{
    auto [succ, hsEntity] = GetHsEntity();
    if (!succ) {
        HILOGE("Failed to get entity of HashStream");
        return FsResult<void>::Error(EIO);
    }

    switch (hsEntity->algType) {
        case HASH_ALGORITHM_TYPE_MD5:
            MD5_Update(&hsEntity->md5Ctx, buffer.buf, buffer.length);
            break;
        case HASH_ALGORITHM_TYPE_SHA1:
            SHA1_Update(&hsEntity->shaCtx, buffer.buf, buffer.length);
            break;
        case HASH_ALGORITHM_TYPE_SHA256:
            SHA256_Update(&hsEntity->sha256Ctx, buffer.buf, buffer.length);
            break;
        default:
            break;
    }

    return FsResult<void>::Success();
}

FsResult<string> HsHashStream::Digest()
{
    auto [succ, hsEntity] = GetHsEntity();
    if (!succ) {
        HILOGE("Failed to get entity of HashStream");
        return FsResult<string>::Error(EIO);
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
    return FsResult<string>::Success(digestStr);
}

FsResult<HsHashStream *> HsHashStream::Constructor(string alg)
{
    HASH_ALGORITHM_TYPE algType = GetHashAlgorithm(alg);
    if (algType == HASH_ALGORITHM_TYPE_UNSUPPORTED) {
        HILOGE("algType is not found.");
        return FsResult<HsHashStream *>::Error(EINVAL);
    }

    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    if (rawPtr == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<HsHashStream *>::Error(ENOMEM);
    }
    std::unique_ptr<HsHashStreamEntity> hsEntity(rawPtr);
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

    HsHashStream *hsStreamPtr = new HsHashStream(move(hsEntity));
    if (hsStreamPtr == nullptr) {
        HILOGE("Failed to create HsHashStream object on heap.");
        return FsResult<HsHashStream *>::Error(ENOMEM);
    }

    return FsResult<HsHashStream *>::Success(move(hsStreamPtr));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS