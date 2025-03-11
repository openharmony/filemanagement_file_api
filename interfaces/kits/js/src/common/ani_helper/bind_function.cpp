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

#include "bind_function.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
ANI_EXPORT ani_status BindClass(ani_vm *vm, const char *className, const std::vector<ani_native_function> &methods)
{
    if (vm == nullptr) {
        HILOGE("ani_vm is null!");
        return ANI_ERROR;
    }

    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        HILOGE("Unsupported ANI_VERSION_1!");
        return ANI_OUT_OF_REF;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Not found '%{private}s'", className);
        return ANI_INVALID_ARGS;
    }

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", className);
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}

ANI_EXPORT ani_status BindNamespace(
    ani_vm *vm, const char *namespaceStr, const std::vector<ani_native_function> &functions)
{
    if (vm == nullptr) {
        HILOGE("ani_vm is null!");
        return ANI_ERROR;
    }

    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        HILOGE("Unsupported ANI_VERSION_1 Fail!!!");
        return ANI_OUT_OF_REF;
    }

    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(namespaceStr, &ns)) {
        HILOGE("Not found '%{private}s'", namespaceStr);
        return ANI_INVALID_ARGS;
    }

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size())) {
        HILOGE("Cannot bind native methods to '%{private}s'", namespaceStr);
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
