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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_ANI_TASK_SIGNAL_LISTENER_ANI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_ANI_TASK_SIGNAL_LISTENER_ANI_H

#include <ani.h>
#include "task_signal_listener.h"
#include "task_signal.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace DistributedFS::ModuleTaskSignal;

class TaskSignalListenerAni : public TaskSignalListener {
public:
    TaskSignalListenerAni(ani_vm *vm, const ani_object &signalObject, std::shared_ptr<TaskSignal> taskSignal)
        : vm(vm), signalObj(signalObject), taskSignal(taskSignal)
    {
        CreateGlobalReference();
    }
    void OnCancel() override;

public:
    TaskSignalListenerAni() = default;
    ~TaskSignalListenerAni();

private:
    void SendCancelEvent(const std::string &filepath) const;
    bool CreateGlobalReference();

private:
    ani_vm *vm;
    ani_object signalObj;
    ani_ref signalRef = nullptr;
    std::shared_ptr<TaskSignal> taskSignal;
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_TASKSIGNAL_ANI_TASK_SIGNAL_LISTENER_ANI_H
