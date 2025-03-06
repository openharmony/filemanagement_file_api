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

#include "truncate_ani.h"

#include <string>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "truncate_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

static tuple<bool, FileInfo> ParseFileInfo(ani_env *env, ani_object obj, ani_object union_obj)
{
    ani_class stringClass;
    env->FindClass("Lstd/core/String;", &stringClass);
    
    ani_class IntClass;
    env->FindClass("Lstd/core/Int;", &IntClass);

    ani_boolean isPath;
    if (ANI_OK != env->Object_InstanceOf(union_obj, stringClass, &isPath)) {
        HILOGE("Get path faild");
    }
    if (isPath) {
        auto [succ, pathString] = TypeConverter::ToUTF8StringPath(env, static_cast<ani_string>(union_obj));
        if (!succ) {
            HILOGE("ToUTF8StringPath faild");
            return { false, FileInfo { false, {}, {} } };
        }
        auto path = std::make_unique<char[]>(pathString.length() + 1);
        std::strcpy(path.get(), pathString.c_str());
        return { true, FileInfo { true, move(path), {} } };
    }

    ani_boolean isFd;
    if (ANI_OK != env->Object_InstanceOf(union_obj, IntClass, &isFd)) {
        HILOGE("Get fd faild");
    }
    if (isFd) {
        ani_int fd;
        if (ANI_OK != env->Object_CallMethodByName_Int(union_obj, "intValue", nullptr, &fd)) {
            HILOGE("Object_CallMethodByName_Int faild");
        }
        if (fd < 0) {
            HILOGE("Invalid fd");
            return { false, FileInfo { false, {}, {} } };
        }
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
        if (fdg == nullptr) {
            HILOGE("Failed to request heap memory.");
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, move(fdg) } };
    }
    return { false, FileInfo { false, {}, {} } };
}


static std::tuple<bool, std::optional<int64_t>> OptionalInt64(ani_env *env, const ani_object &length)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(length, &isUndefined);
    if (isUndefined)
    {
        HILOGE("Reference_IsUndefined int64 faild");
        return { true, std::nullopt };
    }
  
    ani_long len = 0;
    if (ANI_OK !=env->Object_CallMethodByName_Long(length, "longValue", nullptr, &len)) {
        HILOGE("Object_CallMethodByName_Long");
        return { false, {} };
    }
    return { true, std::make_optional<int64_t>(std::move(len)) };
}

void TruncateAni::TruncateSync(ani_env *env, ani_object obj, ani_object union_obj, 
                            ani_object length)
{
    auto [succ, fileinfo] = ParseFileInfo(env, obj, union_obj);
    if (!succ) {
        HILOGE("Get fileinfo faild");
    }
    
    auto [succLen, len] = OptionalInt64(env, length);
    if (!succLen) {
        HILOGE("ToDoubleLen faild");
    }

    auto ret = TruncateCore::DoTruncate(fileinfo, len);
    if (!ret.IsSuccess()) {
        HILOGE("Truncate faild");
    }
}

} // ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS