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

#include <ani.h>
#include "ani_signature.h"
#include "bind_function.h"
#include "environment_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static ani_status BindStaticMethods(ani_env *env)
{
    auto classDesc = Impl::EnvironmentImpl::classDesc.c_str();
    std::array methods = {
        ani_native_function {
            "getStorageDataDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetStorageDataDirSync) },
        ani_native_function {
            "getUserDataDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetUserDataDirSync) },
        ani_native_function {
            "getUserDownloadDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetUserDownloadDirSync) },
        ani_native_function {
            "getUserDesktopDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetUserDesktopDirSync) },
        ani_native_function {
            "getUserDocumentDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetUserDocumentDirSync) },
        ani_native_function {
            "getExternalStorageDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetExternalStorageDirSync) },
        ani_native_function {
            "getUserHomeDirSync", nullptr, reinterpret_cast<void *>(EnvironmentAni::GetUserHomeDirSync) },
    };
    return BindClassStaticMethods(env, classDesc, methods);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr) {
        HILOGE("Invalid parameter vm");
        return ANI_INVALID_ARGS;
    }

    if (result == nullptr) {
        HILOGE("Invalid parameter result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        HILOGE("Invalid ani version!");
        return ANI_INVALID_VERSION;
    }

    status = BindStaticMethods(env);
    if (status != ANI_OK) {
        HILOGE("Cannot bind native static methods for environment!");
        return status;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
