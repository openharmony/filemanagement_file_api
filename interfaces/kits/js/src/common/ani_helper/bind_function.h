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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_BIND_FUNCTION_H
#define INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_BIND_FUNCTION_H

#include <vector>
#include <ani.h>
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

template <std::size_t N>
ANI_EXPORT ani_status BindClass(ani_env *env, const char *className, const std::array<ani_native_function, N> &methods)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return ANI_INVALID_ARGS;
    }

    if (className == nullptr) {
        HILOGE("Invalid parameter className");
        return ANI_INVALID_ARGS;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class '%{private}s'", className);
        return ANI_NOT_FOUND;
    }

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", className);
        return ANI_ERROR;
    };
    return ANI_OK;
}

template <std::size_t N>
ANI_EXPORT ani_status BindClassStaticMethods(ani_env *env, const char *className,
                                             const std::array<ani_native_function, N> &methods)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return ANI_INVALID_ARGS;
    }

    if (className == nullptr) {
        HILOGE("Invalid parameter className");
        return ANI_INVALID_ARGS;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class '%{private}s'", className);
        return ANI_NOT_FOUND;
    }

    if (ANI_OK != env->Class_BindStaticNativeMethods(cls, methods.data(), methods.size())) {
        HILOGE("Cannot bind static native methods to '%{private}s'", className);
        return ANI_ERROR;
    }
    return ANI_OK;
}

template <std::size_t N>
ANI_EXPORT ani_status BindNamespace(
    ani_env *env, const char *namespaceStr, const std::array<ani_native_function, N> &methods)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return ANI_INVALID_ARGS;
    }

    if (namespaceStr == nullptr) {
        HILOGE("Invalid parameter namespaceStr");
        return ANI_INVALID_ARGS;
    }

    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(namespaceStr, &ns)) {
        HILOGE("Cannot find namespace '%{private}s'", namespaceStr);
        return ANI_NOT_FOUND;
    }

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", namespaceStr);
        return ANI_ERROR;
    };
    return ANI_OK;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_BIND_FUNCTION_H