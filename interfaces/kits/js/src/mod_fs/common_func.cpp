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

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#include "class_stream/stream_entity.h"
#include "class_stream/stream_n_exporter.h"
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

static tuple<bool, size_t> GetActualLen(napi_env env, size_t bufLen, size_t bufOff, NVal op)
{
    bool succ = false;
    size_t retLen = 0;

    if (op.HasProp("length")) {
        int64_t opLength = 0;
        tie(succ, opLength) = op.GetProp("length").ToInt64();
        if (!succ) {
            HILOGE("Invalid option.length, expect integer");
            NError(EINVAL).ThrowErr(env);
            return { false, 0 };
        }
        if (opLength < 0) {
            retLen = bufLen - bufOff;
        } else if (static_cast<size_t>(opLength) > bufLen - bufOff) {
            HILOGE("Invalid option.length, buffer limit exceeded");
            NError(EINVAL).ThrowErr(env);
            return { false, 0 };
        } else {
            retLen = static_cast<size_t>(opLength);
        }
    } else {
        retLen = bufLen - bufOff;
    }

    return { true, retLen };
}

unsigned int CommonFunc::ConvertJsFlags(unsigned int &flags)
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

NVal CommonFunc::InstantiateStat(napi_env env, struct stat &buf)
{
    napi_value objStat = NClass::InstantiateClass(env, StatNExporter::className_, {});
    if (!objStat) {
        HILOGE("Failed to instantiate stat class");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, objStat);
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    statEntity->stat_ = buf;
    return { env, objStat };
}

NVal CommonFunc::InstantiateStream(napi_env env, unique_ptr<FILE, decltype(&fclose)> fp)
{
    napi_value objStream = NClass::InstantiateClass(env, StreamNExporter::className_, {});
    if (!objStream) {
        HILOGE("INNER BUG. Cannot instantiate stream");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, objStream);
    if (!streamEntity) {
        HILOGE("Cannot instantiate stream because of void entity");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    streamEntity->fp.swap(fp);
    return { env, objStream };
}

void CommonFunc::fs_req_cleanup(uv_fs_t* req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}

string CommonFunc::GetModeFromFlags(unsigned int flags)
{
    const string RDONLY = "r";
    const string WRONLY = "w";
    const string APPEND = "a";
    const string TRUNC = "t";
    string mode = RDONLY;
    mode += (((flags & O_RDWR) == O_RDWR) ? WRONLY : "");
    mode = (((flags & O_WRONLY) == O_WRONLY) ? WRONLY : mode);
    if (mode != RDONLY) {
        mode += ((flags & O_TRUNC) ? TRUNC : "");
        mode += ((flags & O_APPEND) ? APPEND : "");
    }
    return mode;
}

tuple<bool, unique_ptr<char[]>, unique_ptr<char[]>> CommonFunc::GetCopyPathArg(napi_env env,
                                                                               napi_value srcPath,
                                                                               napi_value dstPath)
{
    bool succ = false;
    unique_ptr<char[]> src = nullptr;
    tie(succ, src, ignore) = NVal(env, srcPath).ToUTF8String();
    if (!succ) {
        HILOGE("Failed to convert the src path to UTF-8 string");
        return { false, nullptr, nullptr };
    }

    unique_ptr<char[]> dest = nullptr;
    tie(succ, dest, ignore) = NVal(env, dstPath).ToUTF8String();
    if (!succ) {
        HILOGE("Failed to convert the dest path to UTF-8 string");
        return { false, nullptr, nullptr };
    }
    return make_tuple(true, move(src), move(dest));
}

static tuple<bool, unique_ptr<char[]>, size_t> DecodeString(napi_env env, NVal jsStr, NVal encoding)
{
    if (!jsStr.TypeIs(napi_string)) {
        return { false, nullptr, 0 };
    }

    bool succ = false;
    if (!encoding) {
        return jsStr.ToUTF8String();
    }

    unique_ptr<char[]> encodingBuf = nullptr;
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

tuple<bool, void *, size_t, bool, int64_t> CommonFunc::GetReadArg(napi_env env,
    napi_value readBuf, napi_value option)
{
    size_t retLen = 0;
    int64_t position = 0;
    bool succ = false;
    bool posAssigned = false;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    size_t bufLen = 0;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ || bufLen > UINT_MAX) {
        HILOGE("Invalid arraybuffer");
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

tuple<bool, unique_ptr<char[]>, void *, size_t, bool, int64_t> CommonFunc::GetWriteArg(napi_env env,
    napi_value argWBuf, napi_value argOption)
{
    int64_t retPos = 0;
    size_t bufLen = 0;
    bool hasPos = false;
    bool succ = false;
    void *buf = nullptr;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard = nullptr;
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
    if (bufLen > UINT_MAX) {
        HILOGE("The Size of buffer is too large");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, nullptr, 0, hasPos, retPos} ;
    }
    size_t retLen = 0;
    tie(succ, retLen) = GetActualLen(env, bufLen, 0, op);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    if (op.HasProp("offset")) {
        int64_t position = 0;
        tie(succ, position) = op.GetProp("offset").ToInt64();
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
