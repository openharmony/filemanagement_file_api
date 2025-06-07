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

#include <array>

#include <ani.h>

#include "ani_signature.h"
#include "bind_function.h"
#include "close_ani.h"
#include "file_ani.h"
#include "listfile_ani.h"
#include "mkdir_ani.h"
#include "move_ani.h"
#include "open_ani.h"
#include "read_text_ani.h"
#include "stat_ani.h"
#include "unlink_ani.h"
#include "write_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static ani_status BindFileMethods(ani_env *env)
{
    auto classDesc = FS::FileInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "getParent", nullptr, reinterpret_cast<void *>(FileAni::GetParent) },
        ani_native_function { "lockSync", nullptr, reinterpret_cast<void *>(FileAni::LockSync) },
        ani_native_function { "tryLock", nullptr, reinterpret_cast<void *>(FileAni::TryLock) },
        ani_native_function { "unlock", nullptr, reinterpret_cast<void *>(FileAni::UnLock) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindStatClassMethods(ani_env *env)
{
    auto classDesc = FS::StatInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "isBlockDevice", nullptr, reinterpret_cast<void *>(StatAni::IsBlockDevice) },
        ani_native_function { "isCharacterDevice", nullptr, reinterpret_cast<void *>(StatAni::IsCharacterDevice) },
        ani_native_function { "isDirectory", nullptr, reinterpret_cast<void *>(StatAni::IsDirectory) },
        ani_native_function { "isFIFO", nullptr, reinterpret_cast<void *>(StatAni::IsFIFO) },
        ani_native_function { "isFile", nullptr, reinterpret_cast<void *>(StatAni::IsFile) },
        ani_native_function { "isSocket", nullptr, reinterpret_cast<void *>(StatAni::IsSocket) },
        ani_native_function { "isSymbolicLink", nullptr, reinterpret_cast<void *>(StatAni::IsSymbolicLink) },
    };

    return BindClass(env, classDesc, methods);
}

const static string mkdirCtorSig0 = Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType });
const static string mkdirCtorSig1 =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, BasicTypes::booleanType });

static ani_status BindStaticMethods(ani_env *env)
{
    auto classDesc = Impl::FileIoImpl::classDesc.c_str();

    std::array methods = {
        ani_native_function { "closeSync", nullptr, reinterpret_cast<void *>(CloseAni::CloseSync) },
        ani_native_function { "listFileSync", nullptr, reinterpret_cast<void *>(ListFileAni::ListFileSync) },
        ani_native_function { "mkdirSync", mkdirCtorSig0.c_str(), reinterpret_cast<void *>(MkdirkAni::MkdirSync0) },
        ani_native_function { "mkdirSync", mkdirCtorSig1.c_str(), reinterpret_cast<void *>(MkdirkAni::MkdirSync1) },
        ani_native_function { "moveFileSync", nullptr, reinterpret_cast<void *>(MoveAni::MoveFileSync) },
        ani_native_function { "openSync", nullptr, reinterpret_cast<void *>(OpenAni::OpenSync) },
        ani_native_function { "readTextSync", nullptr, reinterpret_cast<void *>(ReadTextAni::ReadTextSync) },
        ani_native_function { "unlinkSync", nullptr, reinterpret_cast<void *>(UnlinkAni::UnlinkSync) },
        ani_native_function { "writeSync", nullptr, reinterpret_cast<void *>(WriteAni::WriteSync) },
    };
    return BindClass(env, classDesc, methods);
}

static ani_status DoBindMethods(ani_env *env)
{
    ani_status status;
    if ((status = BindStaticMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native static methods for BindStaticMethods!");
        return status;
    };

    if ((status = BindFileMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for file Class");
        return status;
    };

    if ((status = BindStatClassMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for Stat Class!");
        return status;
    };

    return ANI_OK;
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

    status = DoBindMethods(env);
    if (status != ANI_OK) {
        return status;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
