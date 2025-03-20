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

#include "move_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "move_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

void MoveAni::MoveFileSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string src, ani_string dest, ani_object mode)
{
    auto [succSrc, srcPath] = TypeConverter::ToUTF8String(env, src);
    if (!succSrc) {
        HILOGE("Invalid src");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    auto [succDest, destPath] = TypeConverter::ToUTF8String(env, dest);
    if (!succDest) {
        HILOGE("Invalid dest");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    auto [succMode, modeOp] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    auto ret = MoveCore::DoMove(srcPath, destPath, modeOp);
    if (!ret.IsSuccess()) {
        HILOGE("Move failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS