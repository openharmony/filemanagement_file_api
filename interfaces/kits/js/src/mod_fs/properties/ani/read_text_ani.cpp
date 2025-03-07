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

#include "read_text_ani.h"

#include "filemgmt_libhilog.h"
#include "read_text_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIo {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI;

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
    ani_int result_ref_res;
    if (ANI_OK !=
        env->Object_CallMethodByName_Int(static_cast<ani_object>(result_ref), "intValue", ":I", &result_ref_res)) {
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
        return { false, nullopt };
    }
    auto [succ, encoding] = TypeConverter::ToUTF8StringPath(env, (ani_string)encoding_ref);
    if (!succ) {
        HILOGE("Invalid encoding");
        return { false, nullopt };
    }
    return { true, make_optional<string>(move(encoding)) };
}

static tuple<bool, optional<ReadTextOptions>> ToReadTextOptions(ani_env *env, ani_object obj)
{
    ReadTextOptions result;

    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        HILOGE("options is undefined");
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

ani_string ReadTextAni::ReadTextSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string filePath, ani_object obj)
{
    auto [succOpt, options] = ToReadTextOptions(env, obj);
    if (!succOpt) {
        HILOGE("Ivalid options");
        return nullptr;
    }

    auto [succPath, path] = TypeConverter::ToUTF8StringPath(env, filePath);
    if (!succPath) {
        HILOGE("Invalid Path");
        return nullptr;
    }

    auto ret = ReadTextCore::DoReadText(path, options);
    if (!ret.IsSuccess()) {
        HILOGE("DoReadText failed");
        return nullptr;
    }

    const auto &resText = ret.GetData().value();
    string res = std::get<0>(resText);
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIo
} // namespace FileManagement
} // namespace OHOS