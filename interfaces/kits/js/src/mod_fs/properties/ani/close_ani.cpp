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

#include "ani_signature.h"
#include "close_core.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "file_wrapper.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

tuple<bool, int32_t, FsFile*> ParseFdOrFile(ani_env *env, ani_object obj)
{
    int32_t result = -1;
    auto doubleClassDesc = BoxedTypes::Double::classDesc.c_str();
    ani_class doubleClass;
    env->FindClass(doubleClassDesc, &doubleClass);
    ani_boolean isDouble;
    env->Object_InstanceOf(obj, doubleClass, &isDouble);
    if (isDouble) {
        ani_int fd;
        if (ANI_OK != env->Object_CallMethodByName_Int(obj, "toInt", nullptr, &fd)) {
            HILOGE("Get fd value failed");
            return { false, result, nullptr };
        }
        result = static_cast<int32_t>(fd);
        return { true, result, nullptr };
    }

    FsFile *file = FileWrapper::Unwrap(env, obj);
    if (file != nullptr) {
        return { true, -1, file };
    }

    HILOGE("Cannot unwrap fsfile!");
    ErrorHandler::Throw(env, UNKNOWN_ERR);
    return { false, -1, nullptr };
}

void CloseAni::CloseSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_object obj)
{
    auto [succ, fd, file] = ParseFdOrFile(env, obj);
    if (!succ) {
        HILOGE("Parse fd argument failed");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    if (file == nullptr) {
        auto ret = CloseCore::DoClose(fd);
        if (!ret.IsSuccess()) {
            HILOGE("Close %d failed", fd);
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
            return;
        }
        return;
    }

    auto ret = CloseCore::DoClose(file);
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