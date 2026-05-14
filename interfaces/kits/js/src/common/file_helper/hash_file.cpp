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

#include "hash_file.h"

#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sstream>
#include <unistd.h>

namespace OHOS {
namespace DistributedFS {
using namespace std;

static tuple<int, string> HashFinal(int err, const unique_ptr<unsigned char[]> &hashBuf, size_t hashLen)
{
    if (err) {
        return { err, "" };
    }

    stringstream ss;
    for (size_t i = 0; i < hashLen; ++i) {
        const int hexPerByte = 2;
        ss << std::uppercase << std::setfill('0') << std::setw(hexPerByte) << std::hex <<
            static_cast<uint32_t>(hashBuf[i]);
    }

    return { err, ss.str() };
}

static int ForEachFileSegment(const string &fpath, function<void(char *, size_t)> executor)
{
    unique_ptr<FILE, decltype(&fclose)> filp = { fopen(fpath.c_str(), "r"), fclose };
    if (!filp) {
        return errno;
    }

    const size_t pageSize { getpagesize() };
    auto buf = make_unique<char[]>(pageSize);
    size_t actLen;
    do {
        actLen = fread(buf.get(), 1, pageSize, filp.get());
        if (actLen > 0) {
            executor(buf.get(), actLen);
        }
    } while (actLen == pageSize);

    return ferror(filp.get()) ? errno : 0;
}

tuple<int, string> HashFile::HashWithMD5(const string &fpath)
{
    auto res = make_unique<unsigned char[]>(MD5_DIGEST_LENGTH);
    MD5_CTX ctx;
    MD5_Init(&ctx);
    auto md5Update = [ctx = &ctx](char *buf, size_t len) {
        MD5_Update(ctx, buf, len);
    };
    int err = ForEachFileSegment(fpath, md5Update);
    MD5_Final(res.get(), &ctx);
    return HashFinal(err, res, MD5_DIGEST_LENGTH);
}

tuple<int, string> HashFile::HashWithSHA1(const string &fpath)
{
    auto res = make_unique<unsigned char[]>(SHA_DIGEST_LENGTH);
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    auto sha1Update = [ctx = &ctx](char *buf, size_t len) {
        SHA1_Update(ctx, buf, len);
    };
    int err = ForEachFileSegment(fpath, sha1Update);
    SHA1_Final(res.get(), &ctx);
    return HashFinal(err, res, SHA_DIGEST_LENGTH);
}

tuple<int, string> HashFile::HashWithSHA256(const string &fpath)
{
    auto res = make_unique<unsigned char[]>(SHA256_DIGEST_LENGTH);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    auto sha256Update = [ctx = &ctx](char *buf, size_t len) {
        SHA256_Update(ctx, buf, len);
    };
    int err = ForEachFileSegment(fpath, sha256Update);
    SHA256_Final(res.get(), &ctx);
    return HashFinal(err, res, SHA256_DIGEST_LENGTH);
}
} // namespace DistributedFS
} // namespace OHOS