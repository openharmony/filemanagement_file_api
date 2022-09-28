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

#include "common_func.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/log.h"
#include "../common/napi/n_class.h"
#include "../common/napi/n_func_arg.h"
#include "../common/napi/n_val.h"
#include "../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, void *, int> GetActualBuf(napi_env env, void *rawBuf, int64_t bufLen, NVal op)
{
    bool succ = false;
    void *realBuf = nullptr;
    int64_t opOffset = 0;
    if (op.HasProp("offset")) {
        tie(succ, opOffset) = op.GetProp("offset").ToInt64();
        if (!succ || opOffset < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.offset, positive integer is desired");
            return { false, nullptr, opOffset };
        } else if (opOffset > bufLen) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.offset, buffer limit exceeded");
            return { false, nullptr, opOffset };
        } else {
            realBuf = static_cast<uint8_t *>(rawBuf) + opOffset;
        }
    } else {
        realBuf = rawBuf;
    }

    return { true, realBuf, opOffset };
}

static tuple<bool, size_t> GetActualLen(napi_env env, int64_t bufLen, int64_t bufOff, NVal op)
{
    bool succ = false;
    int64_t retLen;
    if (op.HasProp("length")) {
        int64_t opLength;
        tie(succ, opLength) = op.GetProp("length").ToInt64();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.length, expect integer");
            return { false, 0 };
        }

        if (opLength < 0) {
            retLen = (bufLen > bufOff) ? bufLen - bufOff : 0;
        } else if ((bufLen > opLength) && (bufOff > bufLen - opLength)) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.length, buffer limit exceeded");
            return { false, 0 };
        } else {
            retLen = opLength;
        }
    } else {
        retLen = (bufLen > bufOff) ? bufLen - bufOff : 0;
    }

    return { true, retLen };
}

int CommonFunc::ConvertJsFlags(int &flags)
{
    static constexpr int USR_O_RDONLY = 00;
    static constexpr int USR_O_WRONLY = 01;
    static constexpr int USR_O_RDWR = 02;
    static constexpr int USR_O_CREAT = 0100;
    static constexpr int USR_O_EXCL = 0200;
    static constexpr int USR_O_TRUNC = 01000;
    static constexpr int USR_O_APPEND = 02000;
    static constexpr int USR_O_NONBLOCK = 04000;
    static constexpr int USR_O_DIRECTORY = 0200000;
    static constexpr int USR_O_NOFOLLOW = 0400000;
    static constexpr int USR_O_SYNC = 04010000;

    int flagsABI = 0;
    flagsABI |= ((flags & USR_O_RDONLY) == USR_O_RDONLY) ? O_RDONLY : 0;
    flagsABI |= ((flags & USR_O_WRONLY) == USR_O_WRONLY) ? O_WRONLY : 0;
    flagsABI |= ((flags & USR_O_RDWR) == USR_O_RDWR) ? O_RDWR : 0;
    flagsABI |= ((flags & USR_O_CREAT) == USR_O_CREAT) ? O_CREAT : 0;
    flagsABI |= ((flags & USR_O_EXCL) == USR_O_EXCL) ? O_EXCL : 0;
    flagsABI |= ((flags & USR_O_TRUNC) == USR_O_TRUNC) ? O_TRUNC : 0;
    flagsABI |= ((flags & USR_O_APPEND) == USR_O_APPEND) ? O_APPEND : 0;
    flagsABI |= ((flags & USR_O_NONBLOCK) == USR_O_NONBLOCK) ? O_NONBLOCK : 0;
    flagsABI |= ((flags & USR_O_DIRECTORY) == USR_O_DIRECTORY) ? O_DIRECTORY : 0;
    flagsABI |= ((flags & USR_O_NOFOLLOW) == USR_O_NOFOLLOW) ? O_NOFOLLOW : 0;
    flagsABI |= ((flags & USR_O_SYNC) == USR_O_SYNC) ? O_SYNC : 0;
    flags = flagsABI;
    return flagsABI;
}

tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>> CommonFunc::GetCopyPathArg(napi_env env,
    napi_value srcPath,
    napi_value dstPath)
{
    auto [succ, src, unuse] = NVal(env, srcPath).ToUTF8String();
    if (!succ) {
        return { false, nullptr, nullptr };
    }

    auto [res, dest, useless] = NVal(env, dstPath).ToUTF8String();
    if (!res) {
        return { false, nullptr, nullptr };
    }

    return make_tuple(true, move(src), move(dest));
}

tuple<bool, void *, int64_t, bool, int64_t, int> CommonFunc::GetReadArg(napi_env env,
                                                                        napi_value readBuf,
                                                                        napi_value option)
{
    bool posAssigned = false;
    int64_t position = 0;
    NVal txt(env, readBuf);
    auto [resToArraybuffer, buf, bufLen] = txt.ToArraybuffer();
    if (!resToArraybuffer) {
        UniError(EINVAL).ThrowErr(env, "Invalid read buffer, expect arraybuffer");
        return { false, nullptr, 0, posAssigned, position, 0 };
    }

    NVal op = NVal(env, option);
    auto [resGetActualBuf, retBuf, offset] = GetActualBuf(env, buf, bufLen, op);
    if (!resGetActualBuf) {
        return { false, nullptr, 0, posAssigned, position, 0 };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    auto [resGetActualLen, retLen] = GetActualLen(env, bufLen, bufOff, op);
    if (!resGetActualLen) {
        return { false, nullptr, 0, posAssigned, position, 0 };
    }

    if (op.HasProp("position")) {
        auto [resGetProp, pos] = op.GetProp("position").ToInt64();
        if (resGetProp && pos >= 0) {
            posAssigned = true;
            position = pos;
        } else {
            UniError(EINVAL).ThrowErr(env, "option.position shall be positive number");
            return { false, nullptr, 0, posAssigned, position, 0 };
        }
    }

    return { true, retBuf, retLen, posAssigned, position, offset };
}

static tuple<bool, unique_ptr<char[]>, int64_t> DecodeString(napi_env env, NVal jsStr, NVal encoding)
{
    unique_ptr<char[]> buf;
    if (!jsStr.TypeIs(napi_string)) {
        return { false, nullptr, 0 };
    }

    if (!encoding) {
        return jsStr.ToUTF8String();
    }

    auto [resToUTF8String, encodingBuf, unuse] = encoding.ToUTF8String();
    if (!resToUTF8String) {
        return { false, nullptr, 0 };
    }

    string encodingStr(encodingBuf.release());
    if (encodingStr == "utf-8") {
        return jsStr.ToUTF8String();
    } else if (encodingStr == "utf-16") {
        return jsStr.ToUTF16String();
    } else {
        return { false, nullptr, 0 };
    }
}

tuple<bool, unique_ptr<char[]>, void *, int64_t, bool, int64_t> CommonFunc::GetWriteArg(napi_env env,
                                                                                        napi_value argWBuf,
                                                                                        napi_value argOption)
{
    bool hasPos = false;
    int64_t retPos = 0;
    bool succ = false;
    void *buf = nullptr;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    auto [resDecodeString, bufferGuard, bufLen] = DecodeString(env, jsBuffer, op.GetProp("encoding"));
    if (!resDecodeString) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Illegal write buffer or encoding");
            return { false, nullptr, nullptr, 0, false, 0 };
        }
    } else {
        buf = bufferGuard.get();
    }

    auto [resGetActualBuf, retBuf, unused] = GetActualBuf(env, buf, bufLen, op);
    if (!resGetActualBuf) {
        return { false, nullptr, nullptr, 0, false, 0 };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    auto [resGetActualLen, retLen] = GetActualLen(env, bufLen, bufOff, op);
    if (!resGetActualLen) {
        return { false, nullptr, nullptr, 0, false, 0 };
    }

    /* To parse options - Where to begin writing */
    if (op.HasProp("position")) {
        auto [resGetProp, position] = op.GetProp("position").ToInt32();
        if (!resGetProp || position < 0) {
            UniError(EINVAL).ThrowErr(env, "option.position shall be positive number");
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
        hasPos = true;
        retPos = position;
    } else {
        retPos = INVALID_POSITION;
    }

    return { true, move(bufferGuard), retBuf, retLen, hasPos, retPos };
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
