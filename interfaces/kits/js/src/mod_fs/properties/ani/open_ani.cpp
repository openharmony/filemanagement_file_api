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

#include "open_ani.h"

#include "ani_helper.h"
#include "error_handler.h"
#include "file_wrapper.h"
#include "filemgmt_libhilog.h"
#include "open_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;

ani_object OpenAni::OpenSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object mode)
{
    auto [succPath, filePath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [succMode, modeOp] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    FsResult<FsFile *> ret = OpenCore::DoOpen(filePath, modeOp);
    if (!ret.IsSuccess()) {
        HILOGE("Open failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    const FsFile *file = ret.GetData().value();
    auto result = FileWrapper::Wrap(env, move(file));
    if (result == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS