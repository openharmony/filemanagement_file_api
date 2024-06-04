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

#ifndef INTERFACES_KITS_JS_SRC_MOD_HASH_CLASS_HASHSTREAM_HASHSTREAM_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_HASH_CLASS_HASHSTREAM_HASHSTREAM_ENTITY_H

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <unistd.h>

#include "hash.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
struct HashStreamEntity {
    MD5_CTX md5Ctx;
    SHA_CTX shaCtx;
    SHA256_CTX sha256Ctx;
    HASH_ALGORITHM_TYPE algType = HASH_ALGORITHM_TYPE_UNSUPPORTED;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif