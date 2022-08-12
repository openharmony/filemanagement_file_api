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

#include "document_napi.h"

#include "document_n_exporter.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleDocument {
/***********************************************
 * Module export and register
 ***********************************************/
napi_value DocumentExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("choose", Choose),
        DECLARE_NAPI_FUNCTION("show", Show),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

NAPI_MODULE(document, DocumentExport)
} // namespace ModuleDocument
} // namespace DistributedFS
} // namespace OHOS