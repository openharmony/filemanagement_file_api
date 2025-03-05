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

#ifndef INTERFACES_KITS_JS_SRC_MOD_HASH_PROPERTIES_HASH_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_HASH_PROPERTIES_HASH_CORE_H

#include <map>
#include "filemgmt_libfs.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

enum HASH_ALGORITHM_TYPE {
    HASH_ALGORITHM_TYPE_MD5,
    HASH_ALGORITHM_TYPE_SHA1,
    HASH_ALGORITHM_TYPE_SHA256,
    HASH_ALGORITHM_TYPE_UNSUPPORTED,
};

const std::map<std::string, HASH_ALGORITHM_TYPE> algorithmMaps = {
    {"md5", HASH_ALGORITHM_TYPE_MD5},
    {"sha1", HASH_ALGORITHM_TYPE_SHA1},
    {"sha256", HASH_ALGORITHM_TYPE_SHA256},
};

class HashCore final {
public:
    static FsResult<std::string> DoHash(const std::string &path, const std::string &algorithm);
};

const std::string PROCEDURE_HASH_NAME = "FileIOHash";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_HASH_PROPERTIES_HASH_CORE_H