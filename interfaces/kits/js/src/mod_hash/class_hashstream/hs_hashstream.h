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

#ifndef INTERFACES_KITS_JS_SRC_MOD_HASH_CLASS_HASHSTREAM_HS_HASHSTREAM_H
#define INTERFACES_KITS_JS_SRC_MOD_HASH_CLASS_HASHSTREAM_HS_HASHSTREAM_H

#include <cstdint>

#include "filemgmt_libfs.h"
#include "fs_utils.h"
#include "hs_hashstream_entity.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

class HsHashStream {
public:
    HsHashStream(const HsHashStream &) = delete;
    HsHashStream &operator=(const HsHashStream &) = delete;

    tuple<bool, HsHashStreamEntity *> GetHsEntity();
    FsResult<void> Update(ArrayBuffer &buffer);
    FsResult<string> Digest();

    static FsResult<HsHashStream *> Constructor(string alg);
    ~HsHashStream() = default;

private:
    unique_ptr<HsHashStreamEntity> entity;
    explicit HsHashStream(unique_ptr<HsHashStreamEntity> entity) : entity(move(entity)) {}
};

} // namespace OHOS::FileManagement::ModuleFileIO

#endif // INTERFACES_KITS_JS_SRC_MOD_HASH_CLASS_HASHSTREAM_HS_HASHSTREAM_H