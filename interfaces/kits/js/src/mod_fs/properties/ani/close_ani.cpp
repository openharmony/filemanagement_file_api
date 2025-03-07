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

#include "close_ani.h"

#include <fcntl.h>

#include "close_core.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIo {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

tuple<bool, int> AnalyzerFdUnion(ani_env *env, ani_object obj)
{
    int result = -1;
    ani_class IntClass;
    env->FindClass("Lstd/core/Double;", &IntClass);
    ani_boolean isInt;
    env->Object_InstanceOf(obj, IntClass, &isInt);
    if (isInt) {
        ani_int fd;
        if (ANI_OK != env->Object_CallMethodByName_Int(obj, "intValue", nullptr, &fd)) {
            HILOGE("Get fd value failed");
            return { false, result };
        }
        result = static_cast<int>(fd);
        return { true, result };
    }

    ani_class FileClass;
    env->FindClass("Lfile_fs_class/FileInner;", &FileClass);
    ani_boolean isFile;
    env->Object_InstanceOf(obj, FileClass, &isFile);
    if (isFile) {
        ani_int fd;
        if (ANI_OK != env->Object_GetPropertyByName_Int(obj, "fd", &fd)) {
            HILOGE("Get fd in class file failed");
            return { false, result };
        }
        result = static_cast<int>(fd);
        return { true, result };
    }
    return { false, result };
}

ani_int CloseAni::CloseSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_object obj)
{
    auto [succ, fd] = AnalyzerFdUnion(env, obj);
    if (!succ) {
        HILOGE("Invalid arguments");
        return -1;
    }

    auto ret = CloseCore::DoClose(fd);
    if (!ret.IsSuccess()) {
        HILOGE("Close %d failed", fd);
        return -1;
    }
    return 0;
}

} // namespace ANI
} // namespace ModuleFileIo
} // namespace FileManagement
} // namespace OHOS