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

static FsRandomAccessFile *Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap FsRandomAccessFile err: %{private}d", ret);
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
    ani_class cls;
    auto classDesc = BuiltInTypes::String::classDesc.c_str();
    env->FindClass(classDesc, &cls);

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
    ani_class cls;
    auto classDesc = BuiltInTypes::ArrayBuffer::classDesc.c_str();
    env->FindClass(classDesc, &cls);

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }
    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, move(result) };
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
        return static_cast<long>(ret.GetData().value());
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
        return static_cast<long>(ret.GetData().value());
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
    return static_cast<long>(ret.GetData().value());
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

static ani_object CreateReadStreamOptions(ani_env *env, int64_t start, int64_t end)
{
    static const char *className = "L@ohos/file/fs/ReadStreamOptionsInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", ":V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj)) {
        HILOGE("New %s obj Failed", className);
        return nullptr;
    }

    ani_field startField = nullptr;
    ani_field endField = nullptr;
    if (ANI_OK != env->Class_FindField(cls, "start", &startField)) {
        HILOGE("Cannot find start in class %s", className);
        return nullptr;
    }
    if (ANI_OK != env->Class_FindField(cls, "end", &endField)) {
        HILOGE("Cannot find end in class %s", className);
        return nullptr;
    }

    if (start >= 0) {
        env->Object_SetField_Int(obj, startField, start);
    }
    if (end >= 0) {
        env->Object_SetField_Int(obj, endField, end);
    }
    if (obj == nullptr) {
        HILOGE("CreateReadStreamOptions is nullptr");
    }

    return move(obj);
}

static ani_object CreateWriteStreamOptions(ani_env *env, int64_t start, int flags)
{
    static const char *className = "L@ohos/file/fs/WriteStreamOptionsInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", ":V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj)) {
        HILOGE("New %s obj Failed", className);
        return nullptr;
    }

    ani_field modeField = nullptr;
    ani_field startField = nullptr;
    if (ANI_OK != env->Class_FindField(cls, "mode", &modeField)) {
        HILOGE("Cannot find mode in class %s", className);
        return nullptr;
    }
    if (ANI_OK != env->Class_FindField(cls, "start", &startField)) {
        HILOGE("Cannot find start in class %s", className);
        return nullptr;
    }

    env->Object_SetField_Int(obj, modeField, flags);
    if (start >= 0) {
        env->Object_SetField_Int(obj, startField, start);
    }

    return move(obj);
}

static ani_object CreateReadStream(ani_env *env, ani_string filePath, ani_object options)
{
    static const char *className = "L@ohos/file/fs/fileIo/ReadStream;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;L@ohos/file/fs/ReadStreamOptions;:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, filePath, options)) {
        HILOGE("New %s obj Failed", className);
        return nullptr;
    }

    return move(obj);
}

static ani_object CreateWriteStream(ani_env *env, ani_string filePath, ani_object options)
{
    static const char *className = "L@ohos/file/fs/fileIo/WriteStream;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK !=
        env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;L@ohos/file/fs/WriteStreamOptions;:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, filePath, options)) {
        HILOGE("New %s obj Failed", className);
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
        return CreateReadStream(env, filePath, obj);
    }
    if (streamName == WRITE_STREAM_CLASS) {
        ani_object obj = CreateWriteStreamOptions(env, rafEntity->start, flags);
        return CreateWriteStream(env, filePath, obj);
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