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

#include <optional>
#include "ani_helper.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "read_text_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI;

static tuple<bool, optional<ReadTextOptions>> ToReadTextOptions(ani_env *env, ani_object obj)
{
    ReadTextOptions options;

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

    return { true, make_optional<ReadTextOptions>(move(options)) };
}

ani_string ReadTextAni::ReadTextSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string filePath, ani_object obj)
{
    auto [succOpt, options] = ToReadTextOptions(env, obj);
    if (!succOpt) {
        HILOGE("Invalid options");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [succPath, path] = TypeConverter::ToUTF8String(env, filePath);
    if (!succPath) {
        HILOGE("Invalid Path");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto ret = ReadTextCore::DoReadText(path, options);
    if (!ret.IsSuccess()) {
        HILOGE("DoReadText failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const auto &resText = ret.GetData().value();
    string res = std::get<0>(resText);
    size_t size = std::get<1>(resText);
    auto [succ, result] = TypeConverter::ToAniString(env, res, size);
    if (!succ) {
        HILOGE("Convert result to ani string failed");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS