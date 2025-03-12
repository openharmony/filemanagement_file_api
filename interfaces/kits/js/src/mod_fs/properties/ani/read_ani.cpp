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

#include "read_ani.h"

#include "ani_helper.h"
#include "filemgmt_libhilog.h"
#include "read_core.h"
#include "type_converter.h"
#include <optional>

namespace OHOS::FileManagement::ModuleFileIO::ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

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

ani_long ReadAni::ReadSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_int fd, ani_arraybuffer buffer, ani_object options)
{
    auto [succBuf, arrayBuffer] = TypeConverter::ToArrayBuffer(env, buffer);
    if (!succBuf) {
        HILOGE("Failed to resolve arrayBuffer!");
        return -1;
    }

    auto [succOp, op] = ToReadOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
        return -1;
    }

    auto ret = ReadCore::DoRead(fd, arrayBuffer, op);
    if (!ret.IsSuccess()) {
        HILOGE("Read file content failed!");
        return -1;
    }
    return ret.GetData().value();
}
} // namespace OHOS::FileManagement::ModuleFileIO::ANI