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

#ifndef FILEMANAGEMENT_FILE_API_TASK_SIGNAL_ENTITY_CORE_H
#define FILEMANAGEMENT_FILE_API_TASK_SIGNAL_ENTITY_CORE_H

#include "task_signal.h"
#include "task_signal_listener.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace DistributedFS::ModuleTaskSignal;
typedef std::function<void(std::string filePath)> TaskSignalCb;

class CallbackContextCore {
public:
    explicit CallbackContextCore(TaskSignalCb cb) : cb(cb) {}
    ~CallbackContextCore() = default;

    TaskSignalCb cb;
    std::string filePath_;
};

class TaskSignalEntityCore : public TaskSignalListener {
public:
    TaskSignalEntityCore() = default;
    ~TaskSignalEntityCore() override;
    void OnCancel() override;

    std::shared_ptr<TaskSignal> taskSignal_ = nullptr;
    std::shared_ptr<CallbackContextCore> callbackContextCore_ = nullptr;
};
} // namespace OHOS::FileManagement::ModuleFileIO

#endif // FILEMANAGEMENT_FILE_API_TASK_SIGNAL_ENTITY_CORE_H