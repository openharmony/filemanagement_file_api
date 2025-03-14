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

#ifndef FILEMANAGEMENT_ANI_BIND_FUNCTION_H
#define FILEMANAGEMENT_ANI_BIND_FUNCTION_H

#include <vector>
#include <ani.h>
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

ANI_EXPORT ani_status BindClass(ani_vm *vm, const char *className, const std::vector<ani_native_function> &methods);
ANI_EXPORT ani_status BindNamespace(
    ani_vm *vm, const char *namespaceStr, const std::vector<ani_native_function> &functions);

template <std::size_t N>
ANI_EXPORT ani_status BindClass(ani_env *env, const char *className, const std::array<ani_native_function, N> &methods)
{
    if (env == nullptr) {
        HILOGE("ani_env is null!");
        return ANI_ERROR;
    }
    
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class '%{private}s'", className);
        return ANI_INVALID_ARGS;
    }

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", className);
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}

template <std::size_t N>
ANI_EXPORT ani_status BindNamespace(
    ani_env *env, const char *namespaceStr, const std::array<ani_native_function, N> &methods)
{
    if (env == nullptr) {
        HILOGE("ani_env is null!");
        return ANI_ERROR;
    }

    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(namespaceStr, &ns)) {
        HILOGE("Not found '%{private}s'", namespaceStr);
        return ANI_INVALID_ARGS;
    }

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", namespaceStr);
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_ANI_BIND_FUNCTION_H