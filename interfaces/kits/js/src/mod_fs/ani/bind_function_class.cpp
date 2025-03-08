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

#include <vector>

#include <ani.h>
#include "access_ani.h"
#include "bind_function.h"
#include "copy_file_ani.h"
#include "mkdir_ani.h"
#include "move_ani.h"
#include "stat_ani.h"
#include "unlink_ani.h"
#include "rmdir_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;

static ani_status BindStatMethods(ani_vm *vm)
{
    static const char *className = "Lfile_fs_class/StatInner;";

    std::vector<ani_native_function> methods = {
        ani_native_function { "isBlockDevice", ":Z", reinterpret_cast<void *>(StatAni::IsBlockDevice) },
        ani_native_function { "isCharacterDevice", ":Z", reinterpret_cast<void *>(StatAni::IsCharacterDevice) },
        ani_native_function { "isDirectory", ":Z", reinterpret_cast<void *>(StatAni::IsDirectory) },
        ani_native_function { "isFIFO", ":Z", reinterpret_cast<void *>(StatAni::IsFIFO) },
        ani_native_function { "isFile", ":Z", reinterpret_cast<void *>(StatAni::IsFile) },
        ani_native_function { "isSocket", ":Z", reinterpret_cast<void *>(StatAni::IsSocket) },
        ani_native_function { "isSymbolicLink", ":Z", reinterpret_cast<void *>(StatAni::IsSymbolicLink) },
    };

    return BindClass(vm, className, methods);
}

static ani_status BindFileFs(ani_vm *vm)
{
    static const char *className = "Lfile_fs_class/fileIo;";

    std::vector<ani_native_function> functions = {
        ani_native_function { "doAccessSync", nullptr, reinterpret_cast<void *>(AccessAni::AccessSync3) },
        ani_native_function { "unlinkSync", "Lstd/core/String;:I", reinterpret_cast<void *>(UnlinkAni::UnlinkSync) },
        ani_native_function { "mkdirSync", "Lstd/core/String;:I", reinterpret_cast<void *>(MkdirkAni::MkdirSync0) },
        ani_native_function { "mkdirSync", "Lstd/core/String;Z:I", reinterpret_cast<void *>(MkdirkAni::MkdirSync1) },
        ani_native_function { "moveFileSync", "Lstd/core/String;Lstd/core/String;Lstd/core/Int;:V",
            reinterpret_cast<void *>(MoveAni::MoveFileSync) },
        ani_native_function { "copyFileSync", nullptr, reinterpret_cast<void *>(CopyFileAni::CopyFileSync) },
        ani_native_function { "statSync", nullptr, reinterpret_cast<void *>(StatAni::StatSync) },
        ani_native_function { "rmdirSync", "Lstd/core/String;:V", reinterpret_cast<void *>(RmdirAni::RmdirSync) },
    };

    return BindClass(vm, className, functions);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    *result = ANI_VERSION_1;
    ani_status ret = ANI_OK;
    if (ANI_OK != (ret = BindStatMethods(vm))) {
        return ret;
    };

    if (ANI_OK != (ret = BindFileFs(vm))) {
        return ret;
    };
    return ret;
}
