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

#include "randomaccessfile_ani.h"

#include "ani_helper.h"
#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_randomaccessfile.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

const int BUF_SIZE = 1024;
const string READ_STREAM_CLASS = "ReadStream";
const string WRITE_STREAM_CLASS = "WriteStream";
const string OFFSET = "offset";
const string LENGTH = "length";

static FsRandomAccessFile *Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap FsRandomAccessFile err: %{public}d", ret);
        return nullptr;
    }

    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsRandomAccessFile *rafFile = reinterpret_cast<FsRandomAccessFile *>(ptrValue);
    return rafFile;
}

static tuple<bool, optional<ReadOptions>> ToReadOptions(ani_env *env, ani_object obj)
{
    ReadOptions options;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succOffset, offset] = AniHelper::ParseInt64Option(env, obj, "offset");
    if (!succOffset) {
        HILOGE("Illegal option.offset parameter");
        return { false, nullopt };
    }
    options.offset = offset;

    auto [succLength, length] = AniHelper::ParseInt64Option(env, obj, "length");
    if (!succLength) {
        HILOGE("Illegal option.length parameter");
        return { false, nullopt };
    }
    options.length = length;
    return { true, make_optional<ReadOptions>(move(options)) };
}

static tuple<bool, optional<WriteOptions>> ToWriteOptions(ani_env *env, ani_object obj)
{
    WriteOptions options;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succOffset, offset] = AniHelper::ParseInt64Option(env, obj, OFFSET);
    if (!succOffset) {
        HILOGE("Illegal option.offset parameter");
        return { false, nullopt };
    }
    options.offset = offset;

    auto [succLength, length] = AniHelper::ParseInt64Option(env, obj, LENGTH);
    if (!succLength) {
        HILOGE("Illegal option.length parameter");
        return { false, nullopt };
    }
    options.length = length;

    auto [succEncoding, encoding] = AniHelper::ParseEncoding(env, obj);
    if (!succEncoding) {
        HILOGE("Illegal option.encoding parameter");
        return { false, nullopt };
    }
    options.encoding = encoding;
    return { true, make_optional<WriteOptions>(move(options)) };
}

static tuple<bool, ani_string> ParseStringBuffer(ani_env *env, const ani_object &buf)
{
    auto& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BuiltInTypes::String::classDesc);
    if (ANI_OK != ret) {
        HILOGE("Cannot find class %{public}s", BuiltInTypes::String::classDesc.c_str());
        return { false, {} };
    }

    ani_boolean isString;
    env->Object_InstanceOf(buf, cls, &isString);
    if (!isString) {
        return { false, {} };
    }
    auto result = static_cast<ani_string>(buf);
    return { true, move(result) };
}

static tuple<bool, ani_arraybuffer> ParseArrayBuffer(ani_env *env, const ani_object &buf)
{
    auto& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BuiltInTypes::ArrayBuffer::classDesc);
    if (ANI_OK != ret) {
        HILOGE("Cannot find class %{public}s", BuiltInTypes::ArrayBuffer::classDesc.c_str());
        return { false, {} };
    }

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }

    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, move(result) };
}

ani_int RandomAccessFileAni::GetFd(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto res = rafFile->GetFD();
    if (!res.IsSuccess()) {
        HILOGE("GetFD failed!");
        const auto &err = res.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return res.GetData().value();
}

ani_long RandomAccessFileAni::GetFilePointer(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto res = rafFile->GetFPointer();
    if (!res.IsSuccess()) {
        HILOGE("GetFPointer failed!");
        const auto &err = res.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return res.GetData().value();
}

void RandomAccessFileAni::SetFilePointer(ani_env *env, [[maybe_unused]] ani_object object, ani_long fp)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = rafFile->SetFilePointerSync(static_cast<int64_t>(fp));
    if (!ret.IsSuccess()) {
        HILOGE("SetFilePointerSync failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void RandomAccessFileAni::Close(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = rafFile->CloseSync();
    if (!ret.IsSuccess()) {
        HILOGE("Close rafFile failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_long RandomAccessFileAni::WriteSync(
    ani_env *env, [[maybe_unused]] ani_object object, ani_object buf, ani_object options)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto [succOp, op] = ToWriteOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [isString, stringBuffer] = ParseStringBuffer(env, buf);
    if (isString) {
        auto [succBuf, buffer] = TypeConverter::ToUTF8String(env, stringBuffer);
        if (!succBuf) {
            HILOGE("Failed to resolve stringBuffer!");
            ErrorHandler::Throw(env, EINVAL);
            return -1;
        }
        auto ret = rafFile->WriteSync(buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            ErrorHandler::Throw(env, ret.GetError());
            return -1;
        }
        return static_cast<ani_long>(ret.GetData().value());
    }

    auto [isArrayBuffer, arrayBuffer] = ParseArrayBuffer(env, buf);
    if (isArrayBuffer) {
        auto [succBuf, buffer] = TypeConverter::ToArrayBuffer(env, arrayBuffer);
        if (!succBuf) {
            HILOGE("Failed to resolve arrayBuffer!");
            ErrorHandler::Throw(env, EINVAL);
            return -1;
        }
        auto ret = rafFile->WriteSync(buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            ErrorHandler::Throw(env, ret.GetError());
            return -1;
        }
        return static_cast<ani_long>(ret.GetData().value());
    }
    HILOGE("Unsupported buffer type!");
    ErrorHandler::Throw(env, EINVAL);
    return -1;
}

ani_long RandomAccessFileAni::ReadSync(
    ani_env *env, [[maybe_unused]] ani_object object, ani_arraybuffer buf, ani_object options)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto [succBuf, arrayBuffer] = TypeConverter::ToArrayBuffer(env, buf);
    if (!succBuf) {
        HILOGE("Failed to resolve arrayBuffer!");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succOp, op] = ToReadOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto ret = rafFile->ReadSync(arrayBuffer, op);
    if (!ret.IsSuccess()) {
        HILOGE("Read file content failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }
    return static_cast<ani_long>(ret.GetData().value());
}

static ani_string GetFilePath(ani_env *env, const int fd)
{
    auto dstFd = dup(fd);
    if (dstFd < 0) {
        HILOGE("Failed to get valid fd, fail reason: %{public}s, fd: %{public}d", strerror(errno), fd);
        return nullptr;
    }

    string path = "/proc/self/fd/" + to_string(dstFd);
    auto buf = CreateUniquePtr<char[]>(BUF_SIZE);
    int readLinkRes = readlink(path.c_str(), buf.get(), BUF_SIZE);
    if (readLinkRes < 0) {
        close(dstFd);
        return nullptr;
    }

    close(dstFd);
    auto [succ, filePath] = TypeConverter::ToAniString(env, string(buf.get()));
    if (!succ) {
        return nullptr;
    }
    return move(filePath);
}

static ani_object CreateBoxedInt(ani_env *env, int32_t value)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BoxedTypes::Int::classDesc);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(ret, ctor) = aniCache.GetMethod(env, BoxedTypes::Int::classDesc, BoxedTypes::Int::ctorDesc,
        BoxedTypes::Int::ctorSig);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((ret = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", BoxedTypes::Int::classDesc.c_str(), ret);
        return nullptr;
    }

    return obj;
}

static ani_object CreateBoxedLong(ani_env *env, int64_t value)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BoxedTypes::Long::classDesc);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(ret, ctor) = aniCache.GetMethod(env, BoxedTypes::Long::classDesc, BoxedTypes::Long::ctorDesc,
        BoxedTypes::Long::ctorSig);
    if (ret != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((ret = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", BoxedTypes::Long::classDesc.c_str(), ret);
        return nullptr;
    }

    return obj;
}

static ani_object CreateReadStreamOptions(ani_env *env, int64_t start, int64_t end)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [res, cls] = aniCache.GetClass(env, FS::ReadStreamOptionsInner::classDesc);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(res, ctor) = aniCache.GetMethod(env, FS::ReadStreamOptionsInner::classDesc,
        FS::ReadStreamOptionsInner::ctorDesc, FS::ReadStreamOptionsInner::ctorSig0);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((res = env->Object_New(cls, ctor, &obj)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", FS::ReadStreamOptionsInner::classDesc.c_str(), res);
        return nullptr;
    }

    if (start >= 0) {
        auto startValue = CreateBoxedLong(env, start);
        if (startValue == nullptr) {
            HILOGE("Create 'start' field value failed!");
            return nullptr;
        }

        auto ret = AniHelper::SetPropertyValue(env, obj, "start", startValue);
        if (ret != ANI_OK) {
            HILOGE("Set 'start' field value failed! ret: %{public}d", ret);
            return nullptr;
        }
    }

    if (end >= 0) {
        auto endValue = CreateBoxedLong(env, end);
        if (endValue == nullptr) {
            HILOGE("Create 'end' field value failed!");
            return nullptr;
        }

        auto ret = AniHelper::SetPropertyValue(env, obj, "end", endValue);
        if (ret != ANI_OK) {
            HILOGE("Set 'end' field value failed! ret: %{public}d", ret);
            return nullptr;
        }
    }

    return move(obj);
}

static ani_object CreateWriteStreamOptions(ani_env *env, int64_t start, int flags)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [res, cls] = aniCache.GetClass(env, FS::WriteStreamOptionsInner::classDesc);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(res, ctor) = aniCache.GetMethod(env, FS::WriteStreamOptionsInner::classDesc,
        FS::WriteStreamOptionsInner::ctorDesc, FS::WriteStreamOptionsInner::ctorSig0);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((res = env->Object_New(cls, ctor, &obj)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", FS::WriteStreamOptionsInner::classDesc.c_str(), res);
        return nullptr;
    }

    auto flagsValue = CreateBoxedInt(env, flags);
    if (flagsValue == nullptr) {
        HILOGE("Create 'mode' field value failed!");
        return nullptr;
    }
    
    auto ret = AniHelper::SetPropertyValue(env, obj, "mode", flagsValue);
    if (ret != ANI_OK) {
        HILOGE("Set 'mode' field value failed! ret: %{public}d", ret);
        return nullptr;
    }

    if (start >= 0) {
        auto startValue = CreateBoxedLong(env, start);
        if (startValue == nullptr) {
            HILOGE("Create 'start' field value failed!");
            return nullptr;
        }

        ret = AniHelper::SetPropertyValue(env, obj, "start", startValue);
        if (ret != ANI_OK) {
            HILOGE("Set 'start' field value failed! ret: %{public}d", ret);
            return nullptr;
        }
    }

    return obj;
}

static ani_object CreateReadStream(ani_env *env, ani_string filePath, ani_object options)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [res, cls] = aniCache.GetClass(env, FS::ReadStream::classDesc);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(res, ctor) = aniCache.GetMethod(env, FS::ReadStream::classDesc, FS::ReadStream::ctorDesc,
        FS::ReadStream::ctorSig);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((res = env->Object_New(cls, ctor, &obj, filePath, options)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", FS::ReadStream::classDesc.c_str(), res);
        return nullptr;
    }

    return move(obj);
}

static ani_object CreateWriteStream(ani_env *env, ani_string filePath, ani_object options)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [res, cls] = aniCache.GetClass(env, FS::WriteStream::classDesc);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_method ctor;
    tie(res, ctor) = aniCache.GetMethod(env, FS::WriteStream::classDesc, FS::WriteStream::ctorDesc,
        FS::WriteStream::ctorSig);
    if (res != ANI_OK) {
        return nullptr;
    }

    ani_object obj;
    if ((res = env->Object_New(cls, ctor, &obj, filePath, options)) != ANI_OK) {
        HILOGE("New %{public}s obj Failed, err: %{public}d", FS::WriteStream::classDesc.c_str(), res);
        return nullptr;
    }

    return move(obj);
}

static ani_object CreateStream(ani_env *env, const string &streamName, RandomAccessFileEntity *rafEntity, int flags)
{
    ani_string filePath = GetFilePath(env, rafEntity->fd.get()->GetFD());
    if (!filePath) {
        HILOGE("Get file path failed, errno=%{public}d", errno);
        ErrorHandler::Throw(env, errno);
        return nullptr;
    }

    if (streamName == READ_STREAM_CLASS) {
        ani_object obj = CreateReadStreamOptions(env, rafEntity->start, rafEntity->end);
        if (obj == nullptr) {
            HILOGE("Create readstreamoptions failed.");
            ErrorHandler::Throw(env, EINVAL);
            return nullptr;
        }

        ani_object readStream = CreateReadStream(env, filePath, obj);
        if (readStream == nullptr) {
            HILOGE("Create readstream failed.");
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return nullptr;
        }
        return readStream;
    }

    if (streamName == WRITE_STREAM_CLASS) {
        ani_object obj = CreateWriteStreamOptions(env, rafEntity->start, flags);
        if (obj == nullptr) {
            HILOGE("Create writestreamoptions failed.");
            ErrorHandler::Throw(env, EINVAL);
            return nullptr;
        }

        ani_object writeStream = CreateWriteStream(env, filePath, obj);
        if (writeStream == nullptr) {
            HILOGE("Create writestream failed.");
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return nullptr;
        }
        return writeStream;
    }

    return nullptr;
}

ani_object RandomAccessFileAni::GetReadStream(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    auto entity = rafFile->GetRAFEntity();
    if (!entity) {
        HILOGE("Get RandomAccessFileEntity failed!");
        ErrorHandler::Throw(env, EIO);
        return nullptr;
    }

    int flags = fcntl(entity->fd.get()->GetFD(), F_GETFL);
    unsigned int uflags = static_cast<unsigned int>(flags);
    if (((uflags & O_ACCMODE) != O_RDONLY) && ((uflags & O_ACCMODE) != O_RDWR)) {
        HILOGE("Failed to check Permission");
        ErrorHandler::Throw(env, EACCES);
        return nullptr;
    }

    return CreateStream(env, READ_STREAM_CLASS, entity, flags);
}

ani_object RandomAccessFileAni::GetWriteStream(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto rafFile = Unwrap(env, object);
    if (rafFile == nullptr) {
        HILOGE("Cannot unwrap rafFile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    auto entity = rafFile->GetRAFEntity();
    if (!entity) {
        HILOGE("Get RandomAccessFileEntity failed!");
        ErrorHandler::Throw(env, EIO);
        return nullptr;
    }

    int flags = fcntl(entity->fd.get()->GetFD(), F_GETFL);
    unsigned int uflags = static_cast<unsigned int>(flags);
    if (((uflags & O_ACCMODE) != O_WRONLY) && ((uflags & O_ACCMODE) != O_RDWR)) {
        HILOGE("Failed to check Permission");
        ErrorHandler::Throw(env, EACCES);
        return nullptr;
    }

    return CreateStream(env, WRITE_STREAM_CLASS, entity, flags);
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS