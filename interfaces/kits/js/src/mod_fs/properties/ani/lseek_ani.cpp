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

#include "lseek_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "lseek_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;

optional<SeekPos> ParseSeekPos(const optional<int32_t> &whence)
{
    if (!whence.has_value()) {
        return nullopt;
    }

    return make_optional(static_cast<SeekPos>(move(whence.value())));
}

ani_long LseekAni::LseekSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_int fd, ani_long offset, ani_enum_item whence)
{
    auto [succWhence, whenceOp] = TypeConverter::EnumToInt32(env, whence);
    if (!succWhence) {
        HILOGE("Invalid whence");
        ErrorHandler::Throw(env, EINVAL);
        return -1;
    }
    auto pos = ParseSeekPos(whenceOp);

    auto ret = LseekCore::DoLseek(fd, offset, pos);
    if (!ret.IsSuccess()) {
        HILOGE("DoLseek failed!");
        const FsError &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return -1;
    }

    return ret.GetData().value();
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS