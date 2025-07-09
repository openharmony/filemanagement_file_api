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

#include "fs_watcher.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"
#include "fs_watch_entity.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

FsResult<FsWatcher *> FsWatcher::Constructor()
{
    auto watchEntity = CreateUniquePtr<FsWatchEntity>();
    if (watchEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsWatcher *>::Error(ENOMEM);
    }

    FsWatcher *watcherPtr = new FsWatcher(move(watchEntity));

    if (watcherPtr == nullptr) {
        HILOGE("Failed to create FsWatcher object on heap.");
        return FsResult<FsWatcher *>::Error(ENOMEM);
    }

    return FsResult<FsWatcher *>::Success(move(watcherPtr));
}

FsResult<void> FsWatcher::Stop()
{
    if (!watchEntity) {
        HILOGE("Failed to get watchEntity when stop.");
        return FsResult<void>::Error(EINVAL);
    }
    int ret = FsFileWatcher::GetInstance().StopNotify(watchEntity->watherInfo);
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to stopNotify errno:%{public}d", errno);
        return FsResult<void>::Error(ret);
    }
    return FsResult<void>::Success();
}

FsResult<void> FsWatcher::Start()
{
    if (!watchEntity) {
        HILOGE("Failed to get watchEntity when start.");
        return FsResult<void>::Error(EINVAL);
    }

    shared_ptr<WatcherInfo> info = watchEntity->watherInfo;
    int ret = FsFileWatcher::GetInstance().StartNotify(info);
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to startNotify.");
        return FsResult<void>::Error(ret);
    }

    FsFileWatcher::GetInstance().AsyncGetNotifyEvent();

    return FsResult<void>::Success();
}

} // namespace OHOS::FileManagement::ModuleFileIO
