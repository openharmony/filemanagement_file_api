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
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_randomaccessfile.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

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
    env->FindClass("Lstd/core/String;", &cls);

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
    env->FindClass("Lescompat/ArrayBuffer;", &cls);

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }
    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, move(result) };
}

void RandomAccessFileAni::SetFilePointer(ani_env *env, [[maybe_unused]] ani_object object, ani_double fp)
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
        HILOGE("close rafFile failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_double RandomAccessFileAni::WriteSync(ani_env *env, [[maybe_unused]] ani_object object,
                                          ani_object buf, ani_object options)
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
        return static_cast<double>(ret.GetData().value());
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
        return static_cast<double>(ret.GetData().value());
    }
    HILOGE("Unsupported buffer type!");
    ErrorHandler::Throw(env, EINVAL);
    return -1;
}

ani_double RandomAccessFileAni::ReadSync(ani_env *env, [[maybe_unused]] ani_object object,
                                         ani_arraybuffer buf, ani_object options)
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

    auto ret = rafFile-> ReadSync(arrayBuffer, op);
    if (!ret.IsSuccess()) {
        HILOGE("Read file content failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }
    return static_cast<double>(ret.GetData().value());
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS