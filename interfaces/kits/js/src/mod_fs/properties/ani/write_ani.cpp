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

#include <string>

#include "filemgmt_libhilog.h"
#include "type_converter.h"
#include "write_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

static tuple<bool, optional<int64_t>> ParseOptionalInt64Param(ani_env *env, ani_object obj, const string &tag)
{
    ani_boolean isUndefined = true;
    ani_ref result_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &result_ref)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(result_ref, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }
    ani_long result_ref_res;
    if (ANI_OK !=
        env->Object_CallMethodByName_Long(static_cast<ani_object>(result_ref), "longValue", ":J", &result_ref_res)) {
        return { false, nullopt };
    }
    auto result = make_optional<int64_t>(static_cast<int64_t>(result_ref_res));
    return { true, move(result) };
}

static tuple<bool, optional<string>> ParseEncoding(ani_env *env, ani_object obj)
{
    ani_boolean isUndefined;
    ani_ref encoding_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, "encoding", &encoding_ref)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(encoding_ref, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }
    auto [succ, encoding] = TypeConverter::ToUTF8String(env, (ani_string)encoding_ref);
    if (!succ) {
        return { false, nullopt };
    }
    return { true, make_optional<string>(move(encoding)) };
}

static tuple<bool, optional<WriteOptions>> ToWriteOptions(ani_env *env, ani_object obj)
{
    WriteOptions result;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succOffset, offset] = ParseOptionalInt64Param(env, obj, "offset");
    if (!succOffset) {
        HILOGE("Illegal option.offset parameter");
        return { false, nullopt };
    }
    result.offset = offset;

    auto [succLength, length] = ParseOptionalInt64Param(env, obj, "length");
    if (!succLength) {
        HILOGE("Illegal option.length parameter");
        return { false, nullopt };
    }
    result.length = length;

    auto [succEncoding, encoding] = ParseEncoding(env, obj);
    if (!succEncoding) {
        HILOGE("Illegal option.encoding parameter");
        return { false, nullopt };
    }
    result.encoding = encoding;
    return { true, move(result) };
}

static std::tuple<bool, ani_string> ParseBufToString(ani_env *env, const ani_object &buf)
{
    ani_class cls;
    env->FindClass("Lstd/core/String;", &cls);

    ani_boolean isString;
    env->Object_InstanceOf(buf, cls, &isString);
    if (!isString) {
        return { false, {} };
    }
    auto result = static_cast<ani_string>(buf);
    return { true, std::move(result) };
}

static std::tuple<bool, ani_arraybuffer> ParseBufToArrayBuffer(ani_env *env, const ani_object &buf)
{
    ani_class cls;
    env->FindClass("Lescompat/ArrayBuffer;", &cls);

    ani_boolean isArrayBuffer;
    env->Object_InstanceOf(buf, cls, &isArrayBuffer);
    if (!isArrayBuffer) {
        return { false, {} };
    }
    auto result = static_cast<ani_arraybuffer>(buf);
    return { true, std::move(result) };
}

ani_long WriteAni::WriteSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_int fd, ani_object buf, ani_object options)
{
    auto [succOp, op] = ToWriteOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
        return -1;
    }

    auto [isString, stringBuffer] = ParseBufToString(env, buf);
    if (isString) {
        auto [succBuf, buffer] = TypeConverter::ToUTF8String(env, stringBuffer);
        if (!succBuf) {
            HILOGE("Failed to resolve stringBuffer!");
            return -1;
        }
        auto ret = WriteCore::DoWrite(fd, buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            return -1;
        }
        return ret.GetData().value();
    }

    auto [isArrayBuffer, arrayBuffer] = ParseBufToArrayBuffer(env, buf);
    if (isArrayBuffer) {
        auto [succBuf, buffer] = TypeConverter::ToArrayBuffer(env, arrayBuffer);
        if (!succBuf) {
            HILOGE("Failed to resolve arrayBuffer!");
            return -1;
        }
        auto ret = WriteCore::DoWrite(fd, buffer, op);
        if (!ret.IsSuccess()) {
            HILOGE("write buffer failed!");
            return -1;
        }
        return ret.GetData().value();
    }
    HILOGE("Unsupported buffer type!");
    return -1;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS