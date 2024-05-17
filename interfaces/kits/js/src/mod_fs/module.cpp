/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "common_func.h"

#include <memory>
#include <vector>

#include "class_file/file_n_exporter.h"
#include "class_stat/stat_n_exporter.h"
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "class_randomaccessfile/randomaccessfile_n_exporter.h"
#include "class_readeriterator/readeriterator_n_exporter.h"
#include "class_stream/stream_n_exporter.h"
#include "class_tasksignal/task_signal_n_exporter.h"
#include "class_watcher/watcher_n_exporter.h"
#endif
#include "filemgmt_libhilog.h"
#include "properties/prop_n_exporter.h"

using namespace std;

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
static napi_value Export(napi_env env, napi_value exports)
{
    InitAccessModeType(env, exports);
    InitOpenMode(env, exports);
    InitWhenceType(env, exports);
    std::vector<unique_ptr<NExporter>> products;
    products.emplace_back(make_unique<PropNExporter>(env, exports));
    products.emplace_back(make_unique<FileNExporter>(env, exports));
    products.emplace_back(make_unique<StatNExporter>(env, exports));
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    products.emplace_back(make_unique<RandomAccessFileNExporter>(env, exports));
    products.emplace_back(make_unique<ReaderIteratorNExporter>(env, exports));
    products.emplace_back(make_unique<StreamNExporter>(env, exports));
    products.emplace_back(make_unique<WatcherNExporter>(env, exports));
    products.emplace_back(make_unique<TaskSignalNExporter>(env, exports));
#endif
    for (auto &&product : products) {
#ifdef WIN_PLATFORM
        string nExporterName = product->GetNExporterName();
#else
        string nExporterName = product->GetClassName();
#endif
        if (!product->Export()) {
            HILOGE("INNER BUG. Failed to export class %{public}s for module fileio", nExporterName.c_str());
            return nullptr;
        }
    }
    return exports;
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "file.fs",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
