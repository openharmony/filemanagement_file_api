/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "task_signal.h"

#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"
#include "copy/file_copy_manager.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleTaskSignal {
using namespace FileManagement;
int32_t TaskSignal::Cancel()
{
    HILOGD("TaskSignal Cancel in.");
    if (dfsCopyTask_.load()) {
        auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri_, dstUri_, true);
        if (ret != 0) {
            HILOGE("Cancel failed, ret = %{public}d", ret);
            return ret;
        }
        OnCancel();
        return ret;
    } 
    if (remoteTask_.load()) {
        int32_t ret = 0;
        if (sessionName_.empty()) {
            ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri_, dstUri_, true);
        } else {
            ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
                CancelCopyTask(sessionName_);
        }
        HILOGD("taskSignal.cancel sessionName = %{public}s", sessionName_.c_str());
        if (ret != 0) {
            HILOGI("CancelCopyTask failed, ret = %{public}d", ret);
            return ret;
        }
        OnCancel();
        return ret;
    }
    needCancel_.store(true);
    return 0;
}

bool TaskSignal::IsCanceled()
{
    return needCancel_.load() || remoteTask_.load();
}

void TaskSignal::SetTaskSignalListener(TaskSignalListener *signalListener)
{
    if (signalListener_ == nullptr) {
        signalListener_ = std::move(signalListener);
    }
}

void TaskSignal::OnCancel()
{
    if (signalListener_ != nullptr) {
        signalListener_->OnCancel();
    }
}

bool TaskSignal::CheckCancelIfNeed(const std::string &path)
{
    if (!needCancel_.load()) {
        return false;
    }
    OnCancel();
    return true;
}

void TaskSignal::MarkRemoteTask()
{
    remoteTask_.store(true);
}

void TaskSignal::MarkDfsTask()
{
    dfsCopyTask_.store(true);
}

void TaskSignal::SetFileInfoOfRemoteTask(const std::string &sessionName, const std::string &filePath)
{
    HILOGD("SetFileInfoOfRemoteTask sessionName = %{public}s", sessionName.c_str());
    sessionName_ = sessionName;
    filePath_ = filePath;
}

void TaskSignal::SetCopyTaskUri(const std::string &srcUri, const std::string &dstUri)
{
    srcUri_ = srcUri;
    dstUri_ = dstUri;
}
} // namespace ModuleTaskSignal
} // namespace DistributedFS
} // namespace OHOS