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

#include "hash_core.h"

#include <cstring>
#include <string_view>
#include <tuple>

#include "filemgmt_libhilog.h"
#include "hash_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static HASH_ALGORITHM_TYPE GetHashAlgorithm(const string &alg)
{
    return (algorithmMaps.find(alg) != algorithmMaps.end()) ? algorithmMaps.at(alg) : HASH_ALGORITHM_TYPE_UNSUPPORTED;
}

FsResult<string> HashCore::DoHash(const string &path, const string &algorithm)
{
    HASH_ALGORITHM_TYPE algType = GetHashAlgorithm(algorithm);
    if (algType == HASH_ALGORITHM_TYPE_UNSUPPORTED) {
        HILOGE("Invalid algoritm");
        return FsResult<string>::Error(EINVAL);
    }

    int ret = EIO;
    auto arg = make_shared<string>();
    string &res = *arg;
    if (algType == HASH_ALGORITHM_TYPE_MD5) {
        tie(ret, res) = DistributedFS::HashFile::HashWithMD5(path);
    } else if (algType == HASH_ALGORITHM_TYPE_SHA1) {
        tie(ret, res) = DistributedFS::HashFile::HashWithSHA1(path);
    } else if (algType == HASH_ALGORITHM_TYPE_SHA256) {
        tie(ret, res) = DistributedFS::HashFile::HashWithSHA256(path);
    }

    if (ret) {
        return FsResult<string>::Error(ret);
    }

    return FsResult<string>::Success(*arg);
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS