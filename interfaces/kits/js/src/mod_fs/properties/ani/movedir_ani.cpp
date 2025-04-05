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

#include "movedir_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "movedir_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

void MoveDirAni::MoveDirSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string src, ani_string dest, ani_object mode)
{
    auto [succSrc, srcPath] = TypeConverter::ToUTF8String(env, src);
    auto [succDest, destPath] = TypeConverter::ToUTF8String(env, dest);
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [succMode, optMode] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = MoveDirCore::DoMoveDir(srcPath, destPath, optMode);
    if (!ret.fsResult.IsSuccess()) {
        HILOGE("DoCopyFile failed!");
        const FsError &err = ret.fsResult.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS