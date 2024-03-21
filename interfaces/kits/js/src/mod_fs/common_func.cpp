/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "class_stat/stat_entity.h"
#include "class_stat/stat_n_exporter.h"
#ifndef WIN_PLATFORM
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "class_stream/stream_entity.h"
#include "class_stream/stream_n_exporter.h"
#endif
#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"
#include "file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

namespace {
    const std::vector<std::string> PUBLIC_DIR_PATHS = {
        "/Documents"
    };
}

void InitAccessModeType(napi_env env, napi_value exports)
{
    char propertyName[] = "AccessModeType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("EXIST", NVal::CreateInt32(env, MODE_EXIST).val_),
        DECLARE_NAPI_STATIC_PROPERTY("WRITE", NVal::CreateInt32(env, MODE_WRITE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("READ", NVal::CreateInt32(env, MODE_READ).val_),
        DECLARE_NAPI_STATIC_PROPERTY("READ_WRITE", NVal::CreateInt32(env, MODE_READ_WRITE).val_),
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

void InitOpenMode(napi_env env, napi_value exports)
{
    char propertyName[] = "OpenMode";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("READ_ONLY", NVal::CreateInt32(env, USR_READ_ONLY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("WRITE_ONLY", NVal::CreateInt32(env, USR_WRITE_ONLY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("READ_WRITE", NVal::CreateInt32(env, USR_RDWR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("CREATE", NVal::CreateInt32(env, USR_CREATE).val_),
        DECLARE_NAPI_STATIC_PROPERTY("TRUNC", NVal::CreateInt32(env, USR_TRUNC).val_),
        DECLARE_NAPI_STATIC_PROPERTY("APPEND", NVal::CreateInt32(env, USR_APPEND).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NONBLOCK", NVal::CreateInt32(env, USR_NONBLOCK).val_),
        DECLARE_NAPI_STATIC_PROPERTY("DIR", NVal::CreateInt32(env, USR_DIRECTORY).val_),
        DECLARE_NAPI_STATIC_PROPERTY("NOFOLLOW", NVal::CreateInt32(env, USR_NOFOLLOW).val_),
        DECLARE_NAPI_STATIC_PROPERTY("SYNC", NVal::CreateInt32(env, USR_SYNC).val_),
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

void InitWhenceType(napi_env env, napi_value exports)
{
    char propertyName[] = "WhenceType";
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("SEEK_SET", NVal::CreateInt32(env, SEEK_SET).val_),
        DECLARE_NAPI_STATIC_PROPERTY("SEEK_CUR", NVal::CreateInt32(env, SEEK_CUR).val_),
        DECLARE_NAPI_STATIC_PROPERTY("SEEK_END", NVal::CreateInt32(env, SEEK_END).val_),
    };
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        HILOGE("Failed to create object at initializing whenceType");
        return;
    }
    status = napi_define_properties(env, obj, sizeof(desc) / sizeof(desc[0]), desc);
    if (status != napi_ok) {
        HILOGE("Failed to set properties of character at initializing whenceType");
        return;
    }
    status = napi_set_named_property(env, exports, propertyName, obj);
    if (status != napi_ok) {
        HILOGE("Failed to set direction property at initializing whenceType");
        return;
    }
}

static tuple<bool, size_t> GetActualLen(napi_env env, size_t bufLen, size_t bufOff, NVal op)
{
    bool succ = false;
    size_t retLen = bufLen - bufOff;

    if (op.HasProp("length")) {
        int64_t opLength = 0;
#ifdef WIN_PLATFORM
        tie(succ, opLength) = op.GetPropValue("length").ToInt64(static_cast<int64_t>(retLen));
#else
        tie(succ, opLength) = op.GetProp("length").ToInt64(static_cast<int64_t>(retLen));
#endif
        if (!succ || opLength < 0 || static_cast<size_t>(opLength) > retLen) {
            HILOGE("Invalid option.length");
            NError(EINVAL).ThrowErr(env);
            return { false, 0 };
        }
        retLen = static_cast<size_t>(opLength);
    }
    return { true, retLen };
}

unsigned int CommonFunc::ConvertJsFlags(unsigned int &flags)
{
    // default value is usrReadOnly 00
    unsigned int flagsABI = 0;
    flagsABI |= ((flags & USR_WRITE_ONLY) == USR_WRITE_ONLY) ? WRONLY : 0;
    flagsABI |= ((flags & USR_RDWR) == USR_RDWR) ? RDWR : 0;
    flagsABI |= ((flags & USR_CREATE) == USR_CREATE) ? CREATE : 0;
    flagsABI |= ((flags & USR_TRUNC) == USR_TRUNC) ? TRUNC : 0;
    flagsABI |= ((flags & USR_APPEND) == USR_APPEND) ? APPEND : 0;
    flagsABI |= ((flags & USR_NONBLOCK) == USR_NONBLOCK) ? NONBLOCK : 0;
    flagsABI |= ((flags & USR_DIRECTORY) == USR_DIRECTORY) ? DIRECTORY : 0;
    flagsABI |= ((flags & USR_NOFOLLOW) == USR_NOFOLLOW) ? NOFOLLOW : 0;
    flagsABI |= ((flags & USR_SYNC) == USR_SYNC) ? SYNC : 0;
    flags = flagsABI;
    return flagsABI;
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
NVal CommonFunc::InstantiateStat(napi_env env, const uv_stat_t &buf, shared_ptr<FileInfo> fileInfo)
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
    statEntity->fileInfo_ = fileInfo;
    return { env, objStat };
}
#endif

NVal CommonFunc::InstantiateStat(napi_env env, const uv_stat_t &buf)
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

#ifndef WIN_PLATFORM
NVal CommonFunc::InstantiateFile(napi_env env, int fd, const string &pathOrUri, bool isUri)
{
    napi_value objFile = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objFile) {
        HILOGE("Failed to instantiate class");
        NError(EIO).ThrowErr(env);
        close(fd);
        return NVal();
    }

    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        NError(EIO).ThrowErr(env);
        close(fd);
        return NVal();
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return NVal();
    }
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return { env, objFile };
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
#endif

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
    const string readMode = "r";
    const string writeMode = "w";
    const string appendMode = "a";
    const string truncMode = "t";
    string mode = readMode;
    mode += (((flags & O_RDWR) == O_RDWR) ? writeMode : "");
    mode = (((flags & O_WRONLY) == O_WRONLY) ? writeMode : mode);
    if (mode != readMode) {
        mode += ((flags & O_TRUNC) ? truncMode : "");
        mode += ((flags & O_APPEND) ? appendMode : "");
    }
    return mode;
}

bool CommonFunc::CheckPublicDirPath(const std::string &sandboxPath)
{
    for (const std::string &path : PUBLIC_DIR_PATHS) {
        if (sandboxPath.find(path) == 0) {
            return true;
        }
    }
    return false;
}

string CommonFunc::Decode(const std::string &uri)
{
    std::ostringstream outPutStream;
    const int32_t encodeLen = 2;
    size_t index = 0;
    while (index < uri.length()) {
        if (uri[index] == '%') {
            int hex = 0;
            std::istringstream inputStream(uri.substr(index + 1, encodeLen));
            inputStream >> std::hex >> hex;
            outPutStream << static_cast<char>(hex);
            index += encodeLen + 1;
        } else {
            outPutStream << uri[index];
            index++;
        }
    }

    return outPutStream.str();
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
    if (!encoding) {
        return jsStr.ToUTF8String();
    }

    bool succ = false;
    unique_ptr<char[]> encodingBuf = nullptr;
    tie(succ, encodingBuf, ignore) = encoding.ToUTF8String("utf-8");
    if (!succ) {
        HILOGE("Failed to convert encoding to UTF8");
        return { false, nullptr, 0 };
    }

    string_view encodingStr(encodingBuf.get());
    if (encodingStr == "utf-8") {
        return jsStr.ToUTF8String();
    } else if (encodingStr == "utf-16") {
        return jsStr.ToUTF16String();
    } else {
        HILOGE("Failed to recognize the str type");
        return { false, nullptr, 0 };
    }
}

tuple<bool, void *, size_t, int64_t> CommonFunc::GetReadArg(napi_env env,
    napi_value readBuf, napi_value option)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    size_t bufLen = 0;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ || bufLen > UINT_MAX) {
        HILOGE("Invalid arraybuffer");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, retLen, offset };
    }
    NVal op = NVal(env, option);
    tie(succ, retLen) = GetActualLen(env, bufLen, 0, op);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, retLen, offset };
    }
#ifdef WIN_PLATFORM
    if (op.HasProp("offset") && !op.GetPropValue("offset").TypeIs(napi_undefined)) {
        tie(succ, offset) = op.GetPropValue("offset").ToInt64();
#else
    if (op.HasProp("offset") && !op.GetProp("offset").TypeIs(napi_undefined)) {
        tie(succ, offset) = op.GetProp("offset").ToInt64();
#endif
        if (!succ || offset < 0) {
            HILOGE("option.offset shall be positive number");
            NError(EINVAL).ThrowErr(env);
            return { false, nullptr, retLen, offset };
        }
    }
    return { true, buf, retLen, offset };
}

tuple<bool, unique_ptr<char[]>, void *, size_t, int64_t> CommonFunc::GetWriteArg(napi_env env,
    napi_value argWBuf, napi_value argOption)
{
    size_t bufLen = 0;
    int64_t offset = -1;
    bool succ = false;
    void *buf = nullptr;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard = nullptr;
#ifdef WIN_PLATFORM
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetPropValue("encoding"));
#else
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetProp("encoding"));
#endif
    if (!succ) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            HILOGE("Illegal write buffer or encoding");
            NError(EINVAL).ThrowErr(env);
            return { false, nullptr, nullptr, 0, offset };
        }
    } else {
        buf = bufferGuard.get();
    }
    if (bufLen > UINT_MAX) {
        HILOGE("The Size of buffer is too large");
        NError(EINVAL).ThrowErr(env);
        return { false, nullptr, nullptr, 0, offset } ;
    }
    size_t retLen = 0;
    tie(succ, retLen) = GetActualLen(env, bufLen, 0, op);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, nullptr, 0, offset };
    }

#ifdef WIN_PLATFORM
    if (op.HasProp("offset") && !op.GetPropValue("offset").TypeIs(napi_undefined)) {
        tie(succ, offset) = op.GetPropValue("offset").ToInt64();
#else
    if (op.HasProp("offset") && !op.GetProp("offset").TypeIs(napi_undefined)) {
        tie(succ, offset) = op.GetProp("offset").ToInt64();
#endif
        if (!succ || offset < 0) {
            HILOGE("option.offset shall be positive number");
            NError(EINVAL).ThrowErr(env);
            return { false, nullptr, nullptr, 0, offset };
        }
    }
    return { true, move(bufferGuard), buf, retLen, offset };
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
