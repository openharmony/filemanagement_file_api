/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILE_COMMON_FUNC_H
#define INTERFACES_KITS_JS_SRC_MOD_FILE_COMMON_FUNC_H

#include "../common/napi/n_val.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFile {
struct CommonFunc {
    static std::tuple<bool, napi_ref, napi_ref, napi_ref> GetCallbackHandles(napi_env env,
                                                                             napi_value object);
};
} // namespace ModuleFile
} // namespace DistributedFS
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FILE_COMMON_FUNC_H
