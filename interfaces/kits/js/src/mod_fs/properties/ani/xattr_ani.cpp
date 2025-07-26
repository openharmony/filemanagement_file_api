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

#include "xattr_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"
#include "xattr_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

void XattrAni::SetXattrSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_string key, ani_string value)
{
    auto [pathSucc, pathStr] = TypeConverter::ToUTF8String(env, path);
    if (!pathSucc) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [keySucc, keyStr] = TypeConverter::ToUTF8String(env, key);
    if (!keySucc) {
        HILOGE("Invalid xattr key");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [valueSucc, valueStr] = TypeConverter::ToUTF8String(env, value);
    if (!valueSucc) {
        HILOGE("Invalid xattr value");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = XattrCore::DoSetXattr(pathStr, keyStr, valueStr);
    if (!ret.IsSuccess()) {
        HILOGE("DoSetXattr failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_string XattrAni::GetXattrSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_string key)
{
    auto [pathSucc, pathStr] = TypeConverter::ToUTF8String(env, path);
    if (!pathSucc) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [keySucc, keyStr] = TypeConverter::ToUTF8String(env, key);
    if (!keySucc) {
        HILOGE("Invalid xattr key");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto ret = XattrCore::DoGetXattr(pathStr, keyStr);
    if (!ret.IsSuccess()) {
        HILOGE("DoSetXattr failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const auto &res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS