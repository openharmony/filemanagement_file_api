/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FILEMANAGEMENT_FILE_API_TASK_SIGNAL_H
#define FILEMANAGEMENT_FILE_API_TASK_SIGNAL_H

#include <unistd.h>
#include <memory>
#include <string>

#include "task_signal_listener.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleTaskSignal {
class TaskSignal {
public:
    TaskSignal() = default;
    ~TaskSignal() = default;
    int32_t Cancel();
    bool IsCanceled();
    bool CheckCancelIfNeed(const std::string &path);
    void OnCancel(const std::string &path);
    void SetTaskSignalListener(std::shared_ptr<TaskSignalListener> signalListener);
    void MarkRemoteTask();
    void SetFileInfoOfRemoteTask(const std::string &sessionName, const std::string &filePath);
    std::atomic_bool needCancel_{ false };
    std::atomic_bool remoteTask_{ false };
    std::string sessionName_ = std::string("");
    std::string filePath_ = std::string("");
private:
    std::shared_ptr<TaskSignalListener> signalListener_;
};
} // namespace ModuleTaskSignal
} // namespace DistributedFS
} // namespace OHOS
#endif // FILEMANAGEMENT_FILE_API_TASK_SIGNAL_H