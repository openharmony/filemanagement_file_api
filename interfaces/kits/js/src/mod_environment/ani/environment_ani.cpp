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

#include "environment_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "environment_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleEnvironment;

ani_string EnvironmentAni::GetStorageDataDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetStorageDataDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get storage data dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetUserDataDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetUserDataDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get user data dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetUserDownloadDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetUserDownloadDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get user download dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetUserDesktopDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetUserDesktopDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get user desktop dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetUserDocumentDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetUserDocumentDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get user document dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetExternalStorageDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetExternalStorageDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get external storage dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Create ani_string error");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_string EnvironmentAni::GetUserHomeDirSync(ani_env *env, [[maybe_unused]] ani_class clazz)
{
    auto ret = DoGetUserHomeDir();
    if (!ret.IsSuccess()) {
        HILOGE("Get user home dir failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    string res = ret.GetData().value();
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