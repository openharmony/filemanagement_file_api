/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "filemapping_ani.h"

#include "error_handler.h"
#include "filemapping_wrapper.h"
#include "filemgmt_libhilog.h"
#include "fs_filemapping.h"
#include "type_converter.h"
#include <securec.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

void FileMappingAni::SetPosition(ani_env *env, [[maybe_unused]] ani_object object, ani_int position)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->SetPosition(static_cast<size_t>(position));
    if (!ret.IsSuccess()) {
        HILOGE("SetPosition failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

ani_int FileMappingAni::GetPosition(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto ret = mapping->GetPosition();
    if (!ret.IsSuccess()) {
        HILOGE("GetPosition failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

ani_int FileMappingAni::Capacity(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto ret = mapping->Capacity();
    if (!ret.IsSuccess()) {
        HILOGE("Capacity failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

void FileMappingAni::SetLimit(ani_env *env, [[maybe_unused]] ani_object object, ani_int limit)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->SetLimit(static_cast<size_t>(limit));
    if (!ret.IsSuccess()) {
        HILOGE("SetLimit failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

ani_int FileMappingAni::GetLimit(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto ret = mapping->GetLimit();
    if (!ret.IsSuccess()) {
        HILOGE("GetLimit failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

void FileMappingAni::Flip(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->Flip();
    if (!ret.IsSuccess()) {
        HILOGE("Flip failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

ani_int FileMappingAni::Remaining(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto ret = mapping->Remaining();
    if (!ret.IsSuccess()) {
        HILOGE("Remaining failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

ani_int FileMappingAni::Read(ani_env *env, [[maybe_unused]] ani_object object,
    ani_arraybuffer buffer, ani_object length)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    void *buf = nullptr;
    ani_size bufLen = 0;
    if (ANI_OK != env->ArrayBuffer_GetInfo(buffer, &buf, &bufLen) || !buf) {
        HILOGE("Invalid buffer argument");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succLength, lenOpt] = TypeConverter::ToOptionalInt32(env, length);
    size_t readLen = lenOpt.has_value() ? static_cast<size_t>(lenOpt.value()) : bufLen;

    auto ret = mapping->Read(buf, bufLen, readLen);
    if (!ret.IsSuccess()) {
        HILOGE("Read failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

ani_int FileMappingAni::ReadFrom(ani_env *env, [[maybe_unused]] ani_object object,
    ani_int position, ani_arraybuffer buffer, ani_object length)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    void *buf = nullptr;
    ani_size bufLen = 0;
    if (ANI_OK != env->ArrayBuffer_GetInfo(buffer, &buf, &bufLen) || !buf) {
        HILOGE("Invalid buffer argument");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succLength, lenOpt] = TypeConverter::ToOptionalInt32(env, length);
    size_t readLen = lenOpt.has_value() ? static_cast<size_t>(lenOpt.value()) : bufLen;

    auto ret = mapping->ReadFrom(static_cast<size_t>(position), buf, bufLen, readLen);
    if (!ret.IsSuccess()) {
        HILOGE("ReadFrom failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

ani_int FileMappingAni::Write(ani_env *env, [[maybe_unused]] ani_object object,
    ani_arraybuffer data, ani_object length)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    void *buf = nullptr;
    ani_size bufLen = 0;
    if (ANI_OK != env->ArrayBuffer_GetInfo(data, &buf, &bufLen) || !buf) {
        HILOGE("Invalid data argument");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succLength, lenOpt] = TypeConverter::ToOptionalInt32(env, length);
    size_t writeLen = lenOpt.has_value() ? static_cast<size_t>(lenOpt.value()) : bufLen;

    auto ret = mapping->Write(buf, bufLen, writeLen);
    if (!ret.IsSuccess()) {
        HILOGE("Write failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

ani_int FileMappingAni::WriteTo(ani_env *env, [[maybe_unused]] ani_object object,
    ani_int position, ani_arraybuffer data, ani_object length)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    void *buf = nullptr;
    ani_size bufLen = 0;
    if (ANI_OK != env->ArrayBuffer_GetInfo(data, &buf, &bufLen) || !buf) {
        HILOGE("Invalid data argument");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succLength, lenOpt] = TypeConverter::ToOptionalInt32(env, length);
    size_t writeLen = lenOpt.has_value() ? static_cast<size_t>(lenOpt.value()) : bufLen;

    auto ret = mapping->WriteTo(static_cast<size_t>(position), buf, bufLen, writeLen);
    if (!ret.IsSuccess()) {
        HILOGE("WriteTo failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return static_cast<ani_int>(ret.GetData().value());
#else
    return -1;
#endif
}

void FileMappingAni::MsyncSync(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    if (!mapping->CheckValid()) {
        HILOGE("File mapping is invalid");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->Msync(0, mapping->GetEntity()->capacity);
    if (!ret.IsSuccess()) {
        HILOGE("Msync failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

void FileMappingAni::MsyncSyncWith(ani_env *env, [[maybe_unused]] ani_object object,
    ani_int position, ani_int length)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    if (!mapping->CheckValid()) {
        HILOGE("File mapping is invalid");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->Msync(static_cast<size_t>(position), static_cast<size_t>(length));
    if (!ret.IsSuccess()) {
        HILOGE("Msync failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

void FileMappingAni::UnmapSync(ani_env *env, [[maybe_unused]] ani_object object)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto mapping = FileMappingWrapper::Unwrap(env, object);
    if (mapping == nullptr) {
        HILOGE("Cannot unwrap file mapping!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }

    auto ret = mapping->Unmap();
    if (!ret.IsSuccess()) {
        HILOGE("Unmap failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
    }
    // Do not delete mapping here. The cleaner registered with the ANI object
    // holds a separate copy of the pointer and is responsible for final deletion.
    // Deleting here would cause double-free when the cleaner runs during GC.
    env->Object_SetFieldByName_Long(object, "nativePtr", 0);
#else
    HILOGE("mmap is not supported on this platform");
    ErrorHandler::Throw(env, EOPNOTSUPP);
#endif
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
