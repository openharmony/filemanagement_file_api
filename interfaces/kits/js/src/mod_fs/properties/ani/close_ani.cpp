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
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

tuple<bool, int32_t> ParseFd(ani_env *env, ani_object obj)
{
    int32_t result = -1;
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
        result = static_cast<int32_t>(fd);
        return { true, result };
    }

    ani_class FileClass;
    env->FindClass("L@ohos/file/fs/FileInner;", &FileClass);
    ani_boolean isFile;
    env->Object_InstanceOf(obj, FileClass, &isFile);
    if (isFile) {
        ani_double fd;
        if (ANI_OK != env->Object_GetPropertyByName_Double(obj, "fd", &fd)) {
            HILOGE("Get fd in class file failed");
            return { false, result };
        }
        result = static_cast<int32_t>(fd);
        return { true, result };
    }
    HILOGE("Invalid fd type");
    return { false, result };
}

void CloseAni::CloseSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_object obj)
{
    auto [succ, fd] = ParseFd(env, obj);
    if (!succ) {
        HILOGE("Parse fd argument failed");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = CloseCore::DoClose(fd);
    if (!ret.IsSuccess()) {
        HILOGE("Close %d failed", fd);
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS