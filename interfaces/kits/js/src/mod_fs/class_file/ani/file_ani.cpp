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

#include "file_ani.h"

#include "error_handler.h"
#include "file_wrapper.h"
#include "filemgmt_libhilog.h"
#include "fs_file.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

ani_string FileAni::GetParent(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsFile = FileWrapper::Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return {};
    }
    auto ret = fsFile->GetParent();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot get file parent!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return {};
    }
    auto value = ret.GetData().value();
    auto [succ, parent] = TypeConverter::ToAniString(env, value);
    if (!succ) {
        HILOGE("Cannot convert file parent to ani string!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return {};
    }
    return parent;
}

void FileAni::LockSync(ani_env *env, [[maybe_unused]] ani_object object, ani_object exclusive)
{
    ani_boolean isUndefined;
    bool exc = false;
    env->Reference_IsUndefined(exclusive, &isUndefined);
    if (!isUndefined) {
        exc = true;
    }
    auto fsFile = FileWrapper::Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = fsFile->Lock(exc);
    if (!ret.IsSuccess()) {
        HILOGE("Lock file failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void FileAni::TryLock(ani_env *env, [[maybe_unused]] ani_object object, ani_object exclusive)
{
    ani_boolean isUndefined;
    bool exc = false;
    env->Reference_IsUndefined(exclusive, &isUndefined);
    if (!isUndefined) {
        exc = true;
    }
    auto fsFile = FileWrapper::Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = fsFile->TryLock(exc);
    if (!ret.IsSuccess()) {
        HILOGE("TryLock file failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void FileAni::UnLock(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsFile = FileWrapper::Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto ret = fsFile->UnLock();
    if (!ret.IsSuccess()) {
        HILOGE("UnLock file failed!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS