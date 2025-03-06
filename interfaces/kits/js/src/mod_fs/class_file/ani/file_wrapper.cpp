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

#include "file_wrapper.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_file.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace std;

FsFile *FileWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsFile err: %{private}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsFile *file = reinterpret_cast<FsFile *>(ptrValue);
    return file;
}

ani_object FileWrapper::Wrap(ani_env *env, const FsFile *file)
{
    static const char *className = "L@ohos/file/fs/FileInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(file));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", className);
        return nullptr;
    }

    const auto &fdRet = file->GetFD();
    if (!fdRet.IsSuccess()) {
        HILOGE("GetFD Failed!");
        return nullptr;
    }

    const auto &fd = fdRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "fd", static_cast<double>(fd))) {
        HILOGE("Set fd field value failed!");
        return nullptr;
    }

    const auto &pathRet = file->GetPath();
    if (!pathRet.IsSuccess()) {
        HILOGE("GetPath Failed!");
        return nullptr;
    }

    const auto &path = pathRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "path", path)) {
        HILOGE("Set path field value failed!");
        return nullptr;
    }

    const auto &nameRet = file->GetName();
    if (!pathRet.IsSuccess()) {
        HILOGE("GetPath Failed!");
        return nullptr;
    }

    const auto &name = nameRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "name", name)) {
        HILOGE("Set name field value failed!");
        return nullptr;
    }
    return obj;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS