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

#include "access_ani.h"
#include "bind_function.h"
#include "filemgmt_libhilog.h"
#include "close_ani.h"
#include "copy_file_ani.h"
#include "listfile_ani.h"
#include "mkdir_ani.h"
#include "move_ani.h"
#include "open_ani.h"
#include "read_ani.h"
#include "read_text_ani.h"
#include "rmdir_ani.h"
#include "stat_ani.h"
#include "truncate_ani.h"
#include "unlink_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;

static ani_status BindStatClassMethods(ani_env *env)
{
    static const char *className = "Lfile_fs_class/StatInner;";

    std::array methods = {
        ani_native_function { "isBlockDevice", ":Z", reinterpret_cast<void *>(StatAni::IsBlockDevice) },
        ani_native_function { "isCharacterDevice", ":Z", reinterpret_cast<void *>(StatAni::IsCharacterDevice) },
        ani_native_function { "isDirectory", ":Z", reinterpret_cast<void *>(StatAni::IsDirectory) },
        ani_native_function { "isFIFO", ":Z", reinterpret_cast<void *>(StatAni::IsFIFO) },
        ani_native_function { "isFile", ":Z", reinterpret_cast<void *>(StatAni::IsFile) },
        ani_native_function { "isSocket", ":Z", reinterpret_cast<void *>(StatAni::IsSocket) },
        ani_native_function { "isSymbolicLink", ":Z", reinterpret_cast<void *>(StatAni::IsSymbolicLink) },
    };

    return BindClass(env, className, methods);
}

static ani_status BindStaticMethods(ani_env *env)
{
    static const char *className = "Lfile_fs_class/fileIo;";

    std::array methods = {
        ani_native_function { "closeSync", nullptr, reinterpret_cast<void *>(CloseAni::CloseSync) },
        ani_native_function { "copyFileSync", nullptr, reinterpret_cast<void *>(CopyFileAni::CopyFileSync) },
        ani_native_function { "listFileSync", nullptr, reinterpret_cast<void *>(ListFileAni::ListFileSync) },
        ani_native_function { "mkdirSync", "Lstd/core/String;:I", reinterpret_cast<void *>(MkdirkAni::MkdirSync0) },
        ani_native_function { "mkdirSync", "Lstd/core/String;Z:I", reinterpret_cast<void *>(MkdirkAni::MkdirSync1) },
        ani_native_function { "moveFileSync", nullptr, reinterpret_cast<void *>(MoveAni::MoveFileSync) },
        ani_native_function { "openSync", nullptr, reinterpret_cast<void *>(OpenAni::OpenSync) },
        ani_native_function { "readSync", nullptr, reinterpret_cast<void *>(ReadAni::ReadSync) },
        ani_native_function { "readTextSync", nullptr, reinterpret_cast<void *>(ReadTextAni::ReadTextSync) },
        ani_native_function { "rmdirSync", nullptr, reinterpret_cast<void *>(RmdirAni::RmdirSync) },
        ani_native_function { "statSync", nullptr, reinterpret_cast<void *>(StatAni::StatSync) },
        ani_native_function { "truncateSync", nullptr, reinterpret_cast<void *>(TruncateAni::TruncateSync) },
        ani_native_function { "unlinkSync", nullptr, reinterpret_cast<void *>(UnlinkAni::UnlinkSync) },

    };
    return BindClass(env, className, methods);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        HILOGE("Unsupported ANI_VERSION_1");
        return status;
    }

    status = BindStaticMethods(env);
    if (status != ANI_OK) {
        HILOGE("Cannot bind native static methods for fileio!");
        return status;
    };

    status = BindStatClassMethods(env);
    if (status != ANI_OK) {
        HILOGE("Cannot bind native methods for Stat Class!");
        return status;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
