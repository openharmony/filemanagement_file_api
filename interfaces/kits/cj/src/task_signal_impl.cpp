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

#include "task_signal_impl.h"
#include <memory>
#include "macro.h"

namespace OHOS::CJSystemapi {
using namespace DistributedFS::ModuleTaskSignal;

TaskSignalImpl::TaskSignalImpl()
{
    signalEntity = std::make_shared<TaskSignalEntity>();
    if (signalEntity == nullptr) {
        LOGE("Failed to create TaskSignalEntity.");
        return;
    }
    signalEntity->taskSignal_ = std::make_shared<TaskSignal>();
    if (signalEntity->taskSignal_ == nullptr) {
        LOGE("Failed to create native task signal.");
    }
}

TaskSignalEntity::~TaskSignalEntity() {}

void TaskSignalEntity::OnCancel() {}

} // namespace OHOS::CJSystemapi
