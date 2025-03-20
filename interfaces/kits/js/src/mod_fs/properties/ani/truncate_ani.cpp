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

#include "truncate_ani.h"

#include "error_handler.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "truncate_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

void TruncateAni::TruncateSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_object file, ani_object length)
{
    auto [succ, fileinfo] = TypeConverter::ToFileInfo(env, file);
    if (!succ) {
        HILOGE("Invalid fd/path");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [succLen, len] = TypeConverter::ToOptionalInt64(env, length);
    if (!succLen) {
        HILOGE("Invalid truncate length");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = TruncateCore::DoTruncate(fileinfo, len);
    if (!ret.IsSuccess()) {
        HILOGE("Truncate failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS