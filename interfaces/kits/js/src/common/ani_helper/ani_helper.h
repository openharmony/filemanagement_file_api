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

#ifndef FILEMANAGEMENT_ANI_ANI_HELPER_H
#define FILEMANAGEMENT_ANI_ANI_HELPER_H

#include <string>
#include <tuple>

#include <ani.h>

#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
class AniHelper {
public:
    template <typename T>
    static ani_status SetFieldValue(
        ani_env *env, const ani_class &cls, ani_object &obj, const char *fieldName, const T &value)
    {
        ani_field field;
        auto status = env->Class_FindField(cls, fieldName, &field);
        if (status != ANI_OK) {
            return status;
        }
        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t> || std::is_same_v<T, ani_int>) {
            status = env->Object_SetField_Int(obj, field, value);
        } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, ani_long>) {
            status = env->Object_SetField_Long(obj, field, value);
        } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, ani_double>) {
            status = env->Object_SetField_Double(obj, field, value);
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, ani_boolean>) {
            status = env->Object_SetField_Boolean(obj, field, value);
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_SetField_Ref(obj, field, move(aniStr));
        } else if constexpr (std::is_base_of_v<ani_ref, T>) {
            status = env->Object_SetField_Ref(obj, field, value);
        } else {
            return ANI_INVALID_TYPE;
        }
        return status;
    }

    template <typename T>
    static ani_status SetPropertyValue(
        ani_env *env, const ani_class &cls, ani_object &obj, const std::string &property, const T &value)
    {
        ani_method method;
        std::string setter = "<set>" + property;
        auto status = env->Class_FindMethod(cls, setter.c_str(), nullptr, &method);
        if (status != ANI_OK) {
            return status;
        }

        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_CallMethod_Void(obj, method, move(aniStr));
        } else if constexpr (std::is_base_of_v<ani_ref, T> || std::is_same_v<T, int> || std::is_same_v<T, int32_t> ||
                             std::is_same_v<T, ani_int> || std::is_same_v<T, int64_t> || std::is_same_v<T, ani_long> ||
                             std::is_same_v<T, double> || std::is_same_v<T, ani_double> || std::is_same_v<T, bool> ||
                             std::is_same_v<T, ani_boolean>) {
            status = env->Object_CallMethod_Void(obj, method, value);
        } else {
            return ANI_INVALID_TYPE;
        }
        return status;
    }
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI

#endif // FILEMANAGEMENT_ANI_ANI_HELPER_H