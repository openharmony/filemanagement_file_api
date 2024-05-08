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

#ifndef FILEMANAGEMENT_FILE_API_TASK_SIGNAL_LISTENER_H
#define FILEMANAGEMENT_FILE_API_TASK_SIGNAL_LISTENER_H

namespace OHOS {
namespace DistributedFS {
namespace ModuleTaskSignal {
class TaskSignalListener {
public:
    virtual ~TaskSignalListener() = default;
    virtual void OnCancel(const std::string &path) = 0;
};
} // namespace ModuleTaskSignal
} // namespace DistributedFS
} // namespace OHOS
#endif // FILEMANAGEMENT_FILE_API_TASK_SIGNAL_LISTENER_H