/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_FILE_API_TASK_SIGNAL_N_EXPORTER_H
#define FILEMANAGEMENT_FILE_API_TASK_SIGNAL_N_EXPORTER_H

#include <uv.h>

#include "filemgmt_libn.h"
#include "task_signal_entity.h"

namespace OHOS::FileManagement::ModuleFileIO {
class TaskSignalNExporter final : public LibN::NExporter {
public:
    inline static const std::string className_ = "TaskSignal";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value Cancel(napi_env env, napi_callback_info info);
    static napi_value OnCancel(napi_env env, napi_callback_info info);

    TaskSignalNExporter(napi_env env, napi_value exports);
    ~TaskSignalNExporter() override;
};
} // namespace OHOS::FileManagement::ModuleFileIO

#endif // FILEMANAGEMENT_FILE_API_TASK_SIGNAL_N_EXPORTER_H