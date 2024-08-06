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

#ifndef OHOS_FILE_FS_TASKSIGNAL_H
#define OHOS_FILE_FS_TASKSIGNAL_H

#include "cj_common_ffi.h"
#include "file_utils.h"
#include "ffi_remote_data.h"
#include "task_signal.h"
#include "task_signal_listener.h"

namespace OHOS {
namespace CJSystemapi {
class TaskSignalEntity : public DistributedFS::ModuleTaskSignal::TaskSignalListener {
public:
    TaskSignalEntity() = default;
    ~TaskSignalEntity() override;
    void OnCancel() override;
    std::shared_ptr<DistributedFS::ModuleTaskSignal::TaskSignal> taskSignal_ = nullptr;
};

class TaskSignalImpl : public OHOS::FFI::FFIData {
public:
    TaskSignalImpl();
    std::shared_ptr<TaskSignalEntity> signalEntity = nullptr;
};
}
}

#endif // OHOS_FILE_FS_COPY_H
