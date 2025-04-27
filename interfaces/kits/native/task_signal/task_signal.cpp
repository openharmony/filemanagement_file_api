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

#include "task_signal.h"

#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"
#include <sys/xattr.h>

namespace OHOS {
namespace DistributedFS {
namespace ModuleTaskSignal {
using namespace FileManagement;
constexpr int CANCEL_ERR = -3;
const char CANCEL_XATTR_KEY[] = {"user.cancelcopy"};
const std::string MTP_PATH_PREFIX = "/storage/External/mtp";

int32_t TaskSignal::Cancel()
{
    HILOGD("TaskSignal Cancel in.");
    if (remoteTask_.load()) {
        if (sessionName_.empty()) {
            HILOGE("TaskSignal::Cancel sessionName is empty");
            return CANCEL_ERR;
        }
        auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
                CancelCopyTask(sessionName_);
        HILOGD("taskSignal.cancel sessionName = %{public}s", sessionName_.c_str());
        if (ret != 0) {
            HILOGI("CancelCopyTask failed, ret = %{public}d", ret);
            return ret;
        }
        OnCancel();
        return ret;
    } else {
        if (filePath_.rfind(MTP_PATH_PREFIX, 0) != std::string::npos) {
            std::string value = "";
            if (setxattr(filePath_.c_str(), CANCEL_XATTR_KEY, value.c_str(), value.size(), 0) < 0) {
                HILOGE("cancelcopy setxattr fail, errno is %{public}d", errno);
            }
        }
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

void TaskSignal::SetFileInfoOfRemoteTask(const std::string &sessionName, const std::string &filePath)
{
    HILOGD("SetFileInfoOfRemoteTask sessionName = %{public}s", sessionName.c_str());
    sessionName_ = sessionName;
    filePath_ = filePath;
}
} // namespace ModuleTaskSignal
} // namespace DistributedFS
} // namespace OHOS