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

#include "watcher_core.h"

#include <cstring>
#include <fcntl.h>
#include <tuple>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_file_watcher.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static FsResult<FsWatcher *> InstantiateWatcher()
{
    if (FsFileWatcher::GetInstance().GetNotifyId() < 0 && !FsFileWatcher::GetInstance().InitNotify()) {
        HILOGE("Failed to get notifyId or initnotify fail");
        return FsResult<FsWatcher *>::Error(errno);
    }
    FsResult<FsWatcher *> result = FsWatcher::Constructor();
    if (!result.IsSuccess()) {
        HILOGE("Failed to instantiate watcher");
        return FsResult<FsWatcher *>::Error(EIO);
    }
    return result;
}

shared_ptr<WatcherInfo> ToWatcherInfo(
    const string &path, const int32_t events, shared_ptr<IWatcherCallback> callback, int32_t &errCode)
{
    if (events <= 0 || !FsFileWatcher::GetInstance().CheckEventValid(events)) {
        HILOGE("Failed to get watcher event.");
        errCode = EINVAL;
        return nullptr;
    }

    if (!callback) {
        HILOGE("Failed to get callback");
        errCode = EINVAL;
        return nullptr;
    }

    auto info = CreateSharedPtr<WatcherInfo>(callback);
    if (info == nullptr) {
        HILOGE("Failed to request heap memory.");
        errCode = ENOMEM;
        return nullptr;
    }

    info->events = static_cast<uint32_t>(events);
    info->fileName = move(path);
    return info;
}

FsResult<FsWatcher *> WatcherCore::DoCreateWatcher(
    const string &path, const int32_t events, shared_ptr<IWatcherCallback> callback)
{
    int errCode = 0;
    auto info = ToWatcherInfo(path, events, callback, errCode);
    if (errCode != 0) {
        HILOGE("Failed to parse param");
        return FsResult<FsWatcher *>::Error(errCode);
    }

    auto result = InstantiateWatcher();
    if (!result.IsSuccess()) {
        return result;
    }

    const FsWatcher *objWatcher = result.GetData().value();
    if (!objWatcher) {
        HILOGE("Failed to get fsWatcher");
        return FsResult<FsWatcher *>::Error(EIO);
    }

    auto *watchEntity = objWatcher->GetWatchEntity();
    if (!watchEntity) {
        HILOGE("Failed to get watchEntity.");
        delete objWatcher;
        objWatcher = nullptr;
        return FsResult<FsWatcher *>::Error(EIO);
    }

    watchEntity->watherInfo = info;

    bool ret = FsFileWatcher::GetInstance().AddWatcherInfo(info);
    if (!ret) {
        HILOGE("Failed to add watcher info.");
        return FsResult<FsWatcher *>::Error(EINVAL);
    }
    return result;
}
} // namespace OHOS::FileManagement::ModuleFileIO
