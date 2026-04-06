/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_NAPI_LISTFILE_EXT_NAPI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_NAPI_LISTFILE_EXT_NAPI_H

#include <memory>

#include "filemgmt_libn.h"
#include "listfile_ext_core.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;

class ListFileExtNapi {
public:
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Async(napi_env env, napi_callback_info info);
};

class ListFileExtArgs {
public:
    std::vector<std::string> dirents;
};

const std::string LIST_FILE_EXT_PRODUCE_NAME = "fs.listFileExt";

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_NAPI_LISTFILE_EXT_NAPI_H
