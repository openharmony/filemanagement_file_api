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

#include "open_ani.h"

#include <string>
#include "ani_helper.h"
#include "filemgmt_libhilog.h"
#include "open_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;

static ani_object Wrap(ani_env *env, const FsFile *file)
{
    static const char *className = "Lfile_fs_class/FileInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return {};
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return {};
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(file));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", className);
        return {};
    }

    const auto &fdRet = file->GetFD();
    if (!fdRet.IsSuccess()) {
        HILOGE("GetFD Failed!");
        return {};
    }

    const auto &fd = fdRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "fd", fd)) {
        HILOGE("Set fd field value failed!");
        return {};
    }

    const auto &pathRet = file->GetPath();
    if (!pathRet.IsSuccess()) {
        HILOGE("GetPath Failed!");
        return {};
    }

    const auto &path = pathRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "path", path)) {
        HILOGE("Set path field value failed!");
        return {};
    }

    const auto &nameRet = file->GetName();
    if (!pathRet.IsSuccess()) {
        HILOGE("GetPath Failed!");
        return {};
    }

    const auto &name = nameRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "name", name)) {
        HILOGE("Set name field value failed!");
        return {};
    }
    return obj;
}

ani_object OpenAni::OpenSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object mode)
{
    auto [succPath, filePath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        return {};
    }

    auto [succMode, modeOp] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Invalid mode");
        return {};
    }
    FsResult<FsFile *> ret = OpenCore::DoOpen(filePath, modeOp);
    if (!ret.IsSuccess()) {
        HILOGE("Open failed");
        return {};
    }
    const FsFile *file = ret.GetData().value();
    return Wrap(env, move(file));
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS