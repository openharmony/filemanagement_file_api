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

#include "atomicfile_ani.h"

#include <filesystem>

#include "error_handler.h"
#include "file_wrapper.h"
#include "filemgmt_libhilog.h"
#include "fs_atomicfile.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
namespace fs = std::filesystem;
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

const std::string READ_STREAM_CLASS = "ReadStream";
const std::string WRITE_STREAM_CLASS = "WriteStream";
const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

void AtomicFileAni::Constructor(ani_env *env, ani_object obj, ani_string pathObj){
    auto [succ, filePath] = TypeConverter::ToUTF8String(env, pathObj);
    if (!succ) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }

    auto ret = FsAtomicFile::Constructor(filePath);
    if (!ret.IsSuccess()) {
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    if (ANI_OK != env->Object_SetFieldByName_Long(
        obj, "nativePtr", reinterpret_cast<ani_long>(ret.GetData().value()))) {
        HILOGE("Failed to wrap entity for obj AtomicFile");
        ErrorHandler::Throw(env, EIO);
        return;
    }
}

static FsAtomicFile* Unwrap(ani_env *env, ani_object object)
{
    ani_long file;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &file);
    if (ret != ANI_OK) {
        HILOGE("Unwrap file err: %{private}d", ret);
        return nullptr;
    }

    return reinterpret_cast<FsAtomicFile *>(file);
}

ani_string AtomicFileAni::GetPath(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto file = Unwrap(env, object);
    if (file == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    string path = file->GetPath();
    auto [succ, result] = TypeConverter::ToAniString(env, path);
    if (!succ) {
        HILOGE("ToAniString failed");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return result;
}

ani_object AtomicFileAni::GetBaseFile(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    auto ret = aotomicFile->GetBaseFile();
    if (!ret.IsSuccess()) {
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const FsFile *fsFile = ret.GetData().value();
    auto result = FileWrapper::Wrap(env, move(fsFile));
    if (result == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return result;
}

static ani_object CreateReadStream(ani_env *env, ani_string filePath)
{
    static const char *className = "L@ohos/file/fs/fileIo/ReadStream;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, filePath)) {
        HILOGE("New %s obj Failed", className);
        return nullptr;
    }

    return move(obj);
}

static ani_object CreateWriteStream(ani_env *env, ani_string filePath)
{
    static const char *className = "L@ohos/file/fs/fileIo/WriteStream;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK !=
        env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, filePath)) {
        HILOGE("New %s obj Failed", className);
        return nullptr;
    }

    return move(obj);
}

static ani_object CreateStream(
    ani_env *env, [[maybe_unused]] ani_object object, const std::string &streamName, const std::string &fineName)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    auto [succ, filePath] = TypeConverter::ToAniString(env, fineName);
    if (!succ) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    if (streamName == READ_STREAM_CLASS) {
        auto stream = CreateReadStream(env, filePath);
        if (stream == nullptr) {
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return nullptr;
        }
        return move(stream);
    }
    if (streamName == WRITE_STREAM_CLASS) {
        auto stream = CreateWriteStream(env, filePath);
        if (stream == nullptr) {
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return nullptr;
        }
        return move(stream);
    }

    ErrorHandler::Throw(env, UNKNOWN_ERR);
    return nullptr;
}

ani_object AtomicFileAni::OpenRead(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    auto entity = aotomicFile->GetEntity();
    if (entity == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return CreateStream(env, object, READ_STREAM_CLASS, entity->baseFileName);
}

static tuple<ani_status, ani_arraybuffer> WrapBufferData(ani_env *env, unique_ptr<BufferData> bufferData)
{
    ani_arraybuffer externalBuffer = nullptr;

    uint8_t* buffer = bufferData->buffer;
    size_t length = bufferData->length;
    HILOGE("AtomicFile Arraybuffer: %s", buffer);
    ani_status ret = env->CreateArrayBuffer(length, reinterpret_cast<void **>(&buffer), &externalBuffer);
    bufferData.release();

    if (ret != ANI_OK) {
        HILOGE("CreateArrayBufferExternal err: %{private}d", ret);
        return { ret, nullptr };
    }

    return { ANI_OK, externalBuffer };
}

ani_arraybuffer AtomicFileAni::ReadFully(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    auto ret = aotomicFile->ReadFully();
    if (!ret.IsSuccess()) {
        ErrorHandler::Throw(env, ret.GetError());
        return nullptr;
    }

    auto &bufferData = ret.GetData().value();
    uint8_t* buffer = bufferData->buffer;
    size_t length = bufferData->length;
    auto [succ, obj] = TypeConverter::ToAniArrayBuffer(env, buffer, length);
    if (!succ) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return obj;
}

ani_object AtomicFileAni::StartWrite(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return nullptr;
    }

    auto entity = aotomicFile->GetEntity();
    if (entity == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    fs::path filePath = entity->newFileName;
    fs::path parentPath = filePath.parent_path();
    if (access(parentPath.c_str(), F_OK) != 0) {
        HILOGE("Parent directory does not exist, err:%{public}d", errno);
        ErrorHandler::Throw(env, ENOENT);
        return nullptr;
    }

    char *tmpfile = const_cast<char *>(entity->newFileName.c_str());
    if (mkstemp(tmpfile) == -1) {
        HILOGE("Fail to create tmp file err:%{public}d!", errno);
        ErrorHandler::Throw(env, ENOENT);
        return nullptr;
    }

    ani_object writeStream =  CreateStream(env, object, WRITE_STREAM_CLASS, entity->newFileName);
    if (writeStream == nullptr) {
        HILOGE("Failed to create write stream");
        return nullptr;
    }

    return writeStream;
}

void AtomicFileAni::FinishWrite(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto entity = aotomicFile->GetEntity();
    if (entity == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    aotomicFile->FinishWrite();
    return;
}

void AtomicFileAni::FailWrite(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto entity = aotomicFile->GetEntity();
    if (entity == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    aotomicFile->FailWrite();
    return;
}

void AtomicFileAni::Delete(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto aotomicFile = Unwrap(env, object);
    if (aotomicFile == nullptr) {
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }

    auto ret = aotomicFile->Delete();
    if (!ret.IsSuccess()) {
        ErrorHandler::Throw(env, ret.GetError());
        return;
    }

    return;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS