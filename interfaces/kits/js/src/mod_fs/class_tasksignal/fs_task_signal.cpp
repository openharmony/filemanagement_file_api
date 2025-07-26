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

#include "fs_task_signal.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<shared_ptr<FsTaskSignal>> FsTaskSignal::Constructor(
    shared_ptr<TaskSignal> taskSignal, shared_ptr<TaskSignalListener> signalListener)
{
    if (!taskSignal) {
        HILOGE("Invalid taskSignal");
        return FsResult<shared_ptr<FsTaskSignal>>::Error(EINVAL);
    }
    if (!signalListener) {
        HILOGE("Invalid signalListener");
        return FsResult<shared_ptr<FsTaskSignal>>::Error(EINVAL);
    }
    auto copySignal = CreateSharedPtr<FsTaskSignal>();
    if (copySignal == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<shared_ptr<FsTaskSignal>>::Error(ENOMEM);
    }
    copySignal->taskSignal_ = move(taskSignal);
    copySignal->signalListener_ = move(signalListener);
    return FsResult<shared_ptr<FsTaskSignal>>::Success(copySignal);
}

FsResult<void> FsTaskSignal::Cancel()
{
    if (taskSignal_ == nullptr) {
        HILOGE("Failed to get taskSignal");
        return FsResult<void>::Error(EINVAL);
    }
    auto ret = taskSignal_->Cancel();
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to cancel the task.");
        return FsResult<void>::Error(CANCEL_ERR);
    }
    return FsResult<void>::Success();
}

FsResult<void> FsTaskSignal::OnCancel()
{
    if (taskSignal_ == nullptr) {
        HILOGE("Failed to get taskSignal");
        return FsResult<void>::Error(EINVAL);
    }
    taskSignal_->SetTaskSignalListener(signalListener_.get());
    return FsResult<void>::Success();
}

shared_ptr<TaskSignal> FsTaskSignal::GetTaskSignal() const
{
    return taskSignal_;
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS