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

void InitOpenMode(napi_env env, napi_value exports)
{
    char propertyName[] = "OpenMode";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("READ_ONLY", NVal::CreateInt32(env, RDONLY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("WRITE_ONLY", NVal::CreateInt32(env, WRONLY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("READ_WRITE", NVal::CreateInt32(env, RDWR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("CREATE", NVal::CreateInt32(env, CREATE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("TRUNC", NVal::CreateInt32(env, TRUNC).val_),
        DECLARE_NAPI_STATIC_PROPERTY("APPEND", NVal::CreateInt32(env, APPEND).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NONBLOCK", NVal::CreateInt32(env, NONBLOCK).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DIR", NVal::CreateInt32(env, DIRECTORY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NOFOLLOW", NVal::CreateInt32(env, NOFOLLOW).val_),
        DECLARE_NAPI_STATIC_PROPERTY("SYNC", NVal::CreateInt32(env, SYNC).val_),
    };
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, obj);
}

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
            retLen = bufLen - bufOff;
        } else if (opLength > bufLen - bufOff) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.length, buffer limit exceeded");
            return { false, 0 };
        } else {
            retLen = opLength;
        }
    } else {
        retLen = bufLen - bufOff;
    }

    return { true, retLen };
}

static tuple<bool, size_t> GetActualLenV9(napi_env env, int64_t bufLen, int64_t bufOff, NVal op)
{
    bool succ = false;
    int64_t retLen;

    if (op.HasProp("length")) {
        int64_t opLength;
        tie(succ, opLength) = op.GetProp("length").ToInt64();
        if (!succ) {
            HILOGE("Invalid option.length, expect integer");
            UniError(EINVAL).ThrowErr(env);
            return { false, 0 };
        }
        if (opLength < 0) {
            retLen = bufLen - bufOff;
        } else if (opLength > bufLen - bufOff) {
            HILOGE("Invalid option.length, buffer limit exceeded");
            UniError(EINVAL).ThrowErr(env);
            return { false, 0 };
        } else {
            retLen = opLength;
        }
    } else {
        retLen = bufLen - bufOff;
    }

    return { true, retLen };
}

int CommonFunc::ConvertJsFlags(int &flags)
{
    static constexpr unsigned int usrReadOnly = 00;
    static constexpr unsigned int usrWriteOnly = 01;
    static constexpr unsigned int usrReadWrite = 02;
    static constexpr unsigned int usrCreate = 0100;
    static constexpr unsigned int usrExecuteLock = 0200;
    static constexpr unsigned int usrTruncate = 01000;
    static constexpr unsigned int usrAppend = 02000;
    static constexpr unsigned int usrNoneBlock = 04000;
    static constexpr unsigned int usrDirectory = 0200000;
    static constexpr unsigned int usrNoFollowed = 0400000;
    static constexpr unsigned int usrSynchronous = 04010000;

    unsigned int flagsABI = 0;
    flagsABI |= ((flags & usrReadOnly) == usrReadOnly) ? O_RDONLY : 0;
    flagsABI |= ((flags & usrWriteOnly) == usrWriteOnly) ? O_WRONLY : 0;
    flagsABI |= ((flags & usrReadWrite) == usrReadWrite) ? O_RDWR : 0;
    flagsABI |= ((flags & usrCreate) == usrCreate) ? O_CREAT : 0;
    flagsABI |= ((flags & usrExecuteLock) == usrExecuteLock) ? O_EXCL : 0;
    flagsABI |= ((flags & usrTruncate) == usrTruncate) ? O_TRUNC : 0;
    flagsABI |= ((flags & usrAppend) == usrAppend) ? O_APPEND : 0;
    flagsABI |= ((flags & usrNoneBlock) == usrNoneBlock) ? O_NONBLOCK : 0;
    flagsABI |= ((flags & usrDirectory) == usrDirectory) ? O_DIRECTORY : 0;
    flagsABI |= ((flags & usrNoFollowed) == usrNoFollowed) ? O_NOFOLLOW : 0;
    flagsABI |= ((flags & usrSynchronous) == usrSynchronous) ? O_SYNC : 0;
    flags = flagsABI;
    return flagsABI;
}

tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>> CommonFunc::GetCopyPathArg(napi_env env,
                                                                               napi_value srcPath,
                                                                               napi_value dstPath)
{
    bool succ = false;
    unique_ptr<char[]> src;
    tie(succ, src, ignore) = NVal(env, srcPath).ToUTF8String();
    if (!succ) {
        return { false, nullptr, nullptr };
    }

    unique_ptr<char[]> dest;
    tie(succ, dest, ignore) = NVal(env, dstPath).ToUTF8String();
    if (!succ) {
        return { false, nullptr, nullptr };
    }
    return make_tuple(true, move(src), move(dest));
}

tuple<bool, void *, int64_t, bool, int64_t, int> CommonFunc::GetReadArg(napi_env env,
                                                                        napi_value readBuf,
                                                                        napi_value option)
{
    bool succ = false;
    void *retBuf = nullptr;
    int64_t retLen = 0;
    bool posAssigned = false;
    int64_t position = 0;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    int64_t bufLen = 0;
    int offset = 0;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid read buffer, expect arraybuffer");
        return { false, nullptr, 0, posAssigned, position, offset };
    }

    NVal op = NVal(env, option);
    tie(succ, retBuf, offset) = GetActualBuf(env, buf, bufLen, op);
    if (!succ) {
        return { false, nullptr, 0, posAssigned, position, offset };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    tie(succ, retLen) = GetActualLen(env, bufLen, bufOff, op);
    if (!succ) {
        return { false, nullptr, 0, posAssigned, position, offset };
    }

    if (op.HasProp("position")) {
        tie(succ, position) = op.GetProp("position").ToInt64();
        if (succ && position >= 0) {
            posAssigned = true;
        } else {
            UniError(EINVAL).ThrowErr(env, "option.position shall be positive number");
            return { false, nullptr, 0, posAssigned, position, offset };
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

    bool succ = false;
    if (!encoding) {
        return jsStr.ToUTF8String();
    }

    unique_ptr<char[]> encodingBuf;
    tie(succ, encodingBuf, ignore) = encoding.ToUTF8String();
    if (!succ) {
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
    void *retBuf = nullptr;
    int64_t retLen = 0;
    bool hasPos = false;
    int64_t retPos = 0;

    bool succ = false;
    void *buf = nullptr;
    int64_t bufLen = 0;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard;
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetProp("encoding"));
    if (!succ) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Illegal write buffer or encoding");
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
    } else {
        buf = bufferGuard.get();
    }

    tie(succ, retBuf, ignore) = GetActualBuf(env, buf, bufLen, op);
    if (!succ) {
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    tie(succ, retLen) = GetActualLen(env, bufLen, bufOff, op);
    if (!succ) {
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    if (op.HasProp("position")) {
        int32_t position = 0;
        tie(succ, position) = op.GetProp("position").ToInt32();
        if (!succ || position < 0) {
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

tuple<bool, void *, int64_t, bool, int64_t> CommonFunc::GetReadArgV9(napi_env env,
    napi_value readBuf, napi_value option)
{
    bool succ = false;
    int64_t retLen;
    bool posAssigned = false;
    int64_t position;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    int64_t bufLen;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ) {
        HILOGE("Invalid read buffer, expect arraybuffer");
        UniError(EINVAL).ThrowErr(env);
        return { false, nullptr, 0, posAssigned, position };
    }
    NVal op = NVal(env, option);
    tie(succ, retLen) = GetActualLenV9(env, bufLen, 0, op);
    if (!succ) {
        return { false, nullptr, 0, posAssigned, position };
    }

    if (op.HasProp("offset")) {
        tie(succ, position) = op.GetProp("offset").ToInt64();
        if (succ && position >= 0) {
            posAssigned = true;
        } else {
            HILOGE("option.offset shall be positive number");
            UniError(EINVAL).ThrowErr(env);
            return { false, nullptr, 0, posAssigned, position };
        }
    }

    return { true, buf, retLen, posAssigned, position };
}

tuple<bool, unique_ptr<char[]>, void *, int64_t, bool, int64_t> CommonFunc::GetWriteArgV9(napi_env env,
    napi_value argWBuf, napi_value argOption)
{
    int64_t retLen;
    bool hasPos = false;
    int64_t retPos;
    
    bool succ = false;
    void *buf = nullptr;
    int64_t bufLen;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard;
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetProp("encoding"));
    if (!succ) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            HILOGE("Illegal write buffer or encoding");
            UniError(EINVAL).ThrowErr(env);
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
    } else {
        buf = bufferGuard.get();
    }
    tie(succ, retLen) = GetActualLenV9(env, bufLen, 0, op);
    if (!succ) {
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    if (op.HasProp("offset")) {
        int32_t position = 0;
        tie(succ, position) = op.GetProp("offset").ToInt32();
        if (!succ || position < 0) {
            HILOGE("option.offset shall be positive number");
            UniError(EINVAL).ThrowErr(env);
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
        hasPos = true;
        retPos = position;
    } else {
        retPos = INVALID_POSITION;
    }
    return { true, move(bufferGuard), buf, retLen, hasPos, retPos };
}

} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
