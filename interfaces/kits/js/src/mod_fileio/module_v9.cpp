/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <memory>
#include <vector>

#include "../common/log.h"
#include "class_file/file_n_exporter.h"
#include "class_stat_v9/stat_n_exporter_v9.h"
#include "common_func.h"
#include "properties/prop_n_exporter_v9.h"

using namespace std;

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
static napi_value Export(napi_env env, napi_value exports)
{
    InitOpenMode(env, exports);
    std::vector<unique_ptr<NExporter>> products;
    products.emplace_back(make_unique<PropNExporterV9>(env, exports));
    products.emplace_back(make_unique<FileNExporter>(env, exports));
    products.emplace_back(make_unique<StatNExporterV9>(env, exports));

    for (auto &&product : products) {
        if (!product->Export()) {
            HILOGE("INNER BUG. Failed to export class %{public}s for module fileio", product->GetClassName().c_str());
            return nullptr;
        } else {
            HILOGE("Class %{public}s for module fileio has been exported", product->GetClassName().c_str());
        }
    }
    return exports;
}

NAPI_MODULE(fs, Export)
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS