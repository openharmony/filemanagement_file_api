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

#include "write_ani.h"

#include <optional>
#include "ani_helper.h"
#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"
#include "write_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

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
    auto classDesc = BuiltInTypes::String::classDesc.c_str();
    ani_class cls;
    env->FindClass(classDesc, &cls);

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
    auto classDesc = BuiltInTypes::ArrayBuffer::classDesc.c_str();
    ani_class cls;
    env->FindClass(classDesc, &cls);

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }
    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, std::move(result) };
}

ani_double WriteAni::WriteSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_double fd, ani_object buf, ani_object options)
{
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
        auto ret = WriteCore::DoWrite(static_cast<int32_t>(fd), buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
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
        auto ret = WriteCore::DoWrite(static_cast<int32_t>(fd), buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
            return -1;
        }
        return static_cast<double>(ret.GetData().value());
    }
    HILOGE("Unsupported buffer type!");
    ErrorHandler::Throw(env, EINVAL);
    return -1;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS