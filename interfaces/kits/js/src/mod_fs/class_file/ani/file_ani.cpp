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

#include <string>
#include "file_ani.h"
#include "fs_file.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;

static FsFile *Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsFile err: %{public}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsFile *file = reinterpret_cast<FsFile *>(ptrValue);
    return file;
}

ani_string FileAni::GetParent(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsFile = Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        return {};
    }
    auto ret = fsFile->GetParent();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot get file parent!");
        return {};
    }
    auto value = ret.GetData().value();
    auto [succ, parent] = TypeConverter::ToAniString(env, value);
    if (!succ) {
        HILOGE("Cannot convert file parent to ani string!");
        return {};
    }
    return parent;
}

void FileAni::Lock(ani_env *env, [[maybe_unused]] ani_object object, ani_object exclusive)
{
    ani_boolean isUndefined;
    bool exc = false;
    env->Reference_IsUndefined(exclusive, &isUndefined);
    if (!isUndefined) {
        exc = true;
    }
    auto fsFile = Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        return;
    }
    auto ret = fsFile->Lock(exc);
    if (!ret.IsSuccess()) {
        HILOGE("Lock file failed!");
        return;
    }
    HILOGE("Lock file success.");
}

void FileAni::TryLock(ani_env *env, [[maybe_unused]] ani_object object, ani_object exclusive)
{
    ani_boolean isUndefined;
    bool exc = false;
    env->Reference_IsUndefined(exclusive, &isUndefined);
    if (!isUndefined) {
        exc = true;
    }
    auto fsFile = Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        return;
    }
    auto ret = fsFile->TryLock(exc);
    if (!ret.IsSuccess()) {
        HILOGE("TryLock file failed!");
        return;
    }
    HILOGE("TryLock file success.");
}

void FileAni::UnLock(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsFile = Unwrap(env, object);
    if (fsFile == nullptr) {
        HILOGE("Cannot unwrap fsfile!");
        return;
    }
    auto ret = fsFile->UnLock();
    if (!ret.IsSuccess()) {
        HILOGE("UnLock file failed!");
        return;
    }
    HILOGE("UnLock file success.");
}
} // ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS