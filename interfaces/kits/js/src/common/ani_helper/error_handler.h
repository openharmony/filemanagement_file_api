/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ERROR_HANDLER_H
#define INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ERROR_HANDLER_H

#include <cstdint>
#include <optional>
#include <string>
#include <ani.h>
#include "fs_error.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {

class ErrorHandler {
public:
    static void Throw(
        ani_env *env, int32_t code, const std::string &errMsg, const std::optional<ani_object> &errData = std::nullopt);

    static void Throw(ani_env *env, int32_t code, const std::optional<ani_object> &errData = std::nullopt);

    static void Throw(ani_env *env, const FsError &err, const std::optional<ani_object> &errData = std::nullopt);

private:
    static ani_error CreateErrorObj(
        ani_env *env, int32_t code, const std::string &errMsg, const std::optional<ani_object> &errData = std::nullopt);
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI

#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ERROR_HANDLER_H
