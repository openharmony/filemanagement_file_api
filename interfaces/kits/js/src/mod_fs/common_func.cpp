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

#include "common_func.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

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
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        HILOGE("Failed to create object at initializing openMode");
        return;
    }
    status = napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    if (status != napi_ok) {
        HILOGE("Failed to set properties of character at initializing openMode");
        return;
    }
    status = napi_set_named_property(env, exports, propertyName, obj);
    if (status != napi_ok) {
        HILOGE("Failed to set direction property at initializing openMode");
        return;
    }
}

static tuple<bool, size_t> GetActualLen(napi_env env, int64_t bufLen, int64_t bufOff, NVal op)
{
    bool succ = false;
    int64_t retLen;

    if (op.HasProp("length")) {
        int64_t opLength;
        tie(succ, opLength) = op.GetProp("length").ToInt64();
        if (!succ) {
            HILOGE("Invalid option.length, expect integer");
            NError(EINVAL).ThrowErr(env);
            return { false, 0 };
        }
        if (opLength < 0) {
            retLen = bufLen - bufOff;
        } else if (opLength > bufLen - bufOff) {
            HILOGE("Invalid option.length, buffer limit exceeded");
            NError(EINVAL).ThrowErr(env);
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
    bool succ = false;
    unique_ptr<char[]> src;
    tie(succ, src, ignore) = NVal(env, srcPath).ToUTF8String();
    if (!succ) {
        HILOGE("Failed to convert the src path to UTF-8 string");
        return { false, nullptr, nullptr };
    }

    unique_ptr<char[]> dest;
    tie(succ, dest, ignore) = NVal(env, dstPath).ToUTF8String();
    if (!succ) {
        HILOGE("Failed to convert the dest path to UTF-8 string");
        return { false, nullptr, nullptr };
    }
    return make_tuple(true, move(src), move(dest));
}

static tuple<bool, unique_ptr<char[]>, int64_t> DecodeString(napi_env env, NVal jsStr, NVal encoding)
{
    unique_ptr<char[]> buf;
    if (!jsStr.TypeIs(napi_string)) {
        HILOGE("Failed to recognize the type to a string");
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
    string_view encodingStr(encodingBuf.release());
    if (encodingStr == "utf-8") {
        return jsStr.ToUTF8String();
    } else if (encodingStr == "utf-16") {
        return jsStr.ToUTF16String();
    } else {
        HILOGE("Failed to recognize the str type");
        return { false, nullptr, 0 };
    }
}

tuple<bool, void *, int64_t, bool, int64_t> CommonFunc::GetReadArg(napi_env env,
    napi_value readBuf, napi_value option)
{
    int64_t retLen;
    int64_t position;
    bool succ = false;
    bool posAssigned = false;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    int64_t bufLen;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ) {
        HILOGE("Invalid read buffer, expect arraybuffer");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, 0, posAssigned, position };
    }
    NVal op = NVal(env, option);
    tie(succ, retLen) = GetActualLen(env, bufLen, 0, op);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, 0, posAssigned, position };
    }

    if (op.HasProp("offset")) {
        tie(succ, position) = op.GetProp("offset").ToInt64();
        if (succ && position >= 0) {
            posAssigned = true;
        } else {
            HILOGE("option.offset shall be positive number");
            NError(EINVAL).ThrowErr(env);
            return { false, nullptr, 0, posAssigned, position };
        }
    }

    return { true, buf, retLen, posAssigned, position };
}

tuple<bool, unique_ptr<char[]>, void *, int64_t, bool, int64_t> CommonFunc::GetWriteArg(napi_env env,
    napi_value argWBuf, napi_value argOption)
{
    int64_t retLen;
    int64_t retPos;
    int64_t bufLen;
    bool hasPos = false;
    bool succ = false;
    void *buf = nullptr;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard;
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetProp("encoding"));
    if (!succ) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            HILOGE("Illegal write buffer or encoding");
            NError(EINVAL).ThrowErr(env);
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
    } else {
        buf = bufferGuard.get();
    }
    tie(succ, retLen) = GetActualLen(env, bufLen, 0, op);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    if (op.HasProp("offset")) {
        int32_t position = 0;
        tie(succ, position) = op.GetProp("offset").ToInt32();
        if (!succ || position < 0) {
            HILOGE("option.offset shall be positive number");
            NError(EINVAL).ThrowErr(env);
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
} // namespace FileManagement
} // namespace OHOS
