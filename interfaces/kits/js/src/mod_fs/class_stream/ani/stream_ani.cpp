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

#include "stream_ani.h"

#include <optional>
#include <string>

#include "ani_cache.h"
#include "ani_helper.h"
#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_stream.h"
#include "fs_utils.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

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

static std::tuple<bool, ani_string> ParseStringBuffer(ani_env *env, const ani_object &buf)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BuiltInTypes::String::classDesc);
    if (ret != ANI_OK) {
        return { false, {} };
    }

    ani_boolean isString;
    env->Object_InstanceOf(buf, cls, &isString);
    if (!isString) {
        return { false, {} };
    }
    auto result = static_cast<ani_string>(buf);
    return { true, std::move(result) };
}

static std::tuple<bool, ani_arraybuffer> ParseArrayBuffer(ani_env *env, const ani_object &buf)
{
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, BuiltInTypes::ArrayBuffer::classDesc);
    if (ret != ANI_OK) {
        return { false, {} };
    }

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }
    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, std::move(result) };
}

static FsStream *Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsFile err: %{private}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsStream *stream = reinterpret_cast<FsStream *>(ptrValue);
    return stream;
}

void StreamAni::Close(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStream = Unwrap(env, object);
    if (fsStream == nullptr) {
        HILOGE("Cannot unwrap fsStream!");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = fsStream->Close();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot close fsStream!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void StreamAni::Flush(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStream = Unwrap(env, object);
    if (fsStream == nullptr) {
        HILOGE("Cannot unwrap fsStream!");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = fsStream->Flush();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot flush fsStream!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_long StreamAni::Write(ani_env *env, [[maybe_unused]] ani_object object, ani_object buf, ani_object options)
{
    auto fsStream = Unwrap(env, object);
    if (fsStream == nullptr) {
        HILOGE("Cannot unwrap fsStream!");
        return -1;
    }

    auto [succOp, op] = ToWriteOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
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
        auto ret = fsStream->Write(buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write string failed!");
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
            return -1;
        }
        return ret.GetData().value();
    }

    auto [isArrayBuffer, arrayBuffer] = ParseArrayBuffer(env, buf);
    if (isArrayBuffer) {
        auto [succBuf, buffer] = TypeConverter::ToArrayBuffer(env, arrayBuffer);
        if (!succBuf) {
            HILOGE("Failed to resolve arrayBuffer!");
            ErrorHandler::Throw(env, EINVAL);
            return -1;
        }
        auto ret = fsStream->Write(buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
            return -1;
        }
        return ret.GetData().value();
    }

    HILOGE("Unsupported buffer type!");
    ErrorHandler::Throw(env, EINVAL);
    return -1;
}

ani_long StreamAni::Read(ani_env *env, [[maybe_unused]] ani_object object, ani_arraybuffer buffer, ani_object options)
{
    auto fsStream = Unwrap(env, object);
    if (fsStream == nullptr) {
        HILOGE("Cannot unwrap fsStream!");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succBuf, arrayBuffer] = TypeConverter::ToArrayBuffer(env, buffer);
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

    auto ret = fsStream->Read(arrayBuffer, op);
    if (!ret.IsSuccess()) {
        HILOGE("write buffer failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }
    return ret.GetData().value();
}

ani_long StreamAni::Seek(ani_env *env, [[maybe_unused]] ani_object object, ani_long offset, ani_object whence)
{
    auto fsStream = Unwrap(env, object);
    if (fsStream == nullptr) {
        HILOGE("Cannot unwrap fsStream!");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto [succ, typeOpt] = TypeConverter::ToOptionalInt32(env, whence);
    if (!succ) {
        HILOGE("Invalied whence type");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }

    auto ret = fsStream->Seek(static_cast<int64_t>(offset), typeOpt);
    if (!ret.IsSuccess()) {
        HILOGE("seek failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }
    return ret.GetData().value();
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS