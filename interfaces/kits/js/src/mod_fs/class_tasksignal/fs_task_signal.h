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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_FS_TASK_SIGNAL_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_FS_TASK_SIGNAL_H

#include "filemgmt_libfs.h"
#include "task_signal.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace DistributedFS::ModuleTaskSignal;

class FsTaskSignal {
public:
    static FsResult<shared_ptr<FsTaskSignal>> Constructor(
        shared_ptr<TaskSignal> taskSignal, shared_ptr<TaskSignalListener> signalListener);
    FsResult<void> Cancel();
    FsResult<void> OnCancel();
    shared_ptr<TaskSignal> GetTaskSignal() const;

public:
    FsTaskSignal() = default;
    ~FsTaskSignal() = default;
    FsTaskSignal(const FsTaskSignal &other) = delete;
    FsTaskSignal &operator=(const FsTaskSignal &other) = delete;

private:
    shared_ptr<TaskSignal> taskSignal_ = nullptr;
    shared_ptr<TaskSignalListener> signalListener_ = nullptr;
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_FS_TASK_SIGNAL_H