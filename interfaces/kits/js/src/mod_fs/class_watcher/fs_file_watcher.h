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
#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_FILE_WATCHER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_FILE_WATCHER_H

#include <mutex>
#include <string>
#include <thread>

#include <sys/inotify.h>

#include "filemgmt_libfs.h"
#include "fs_watch_entity.h"
#include "singleton.h"
#include "watcher_data_cache.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

constexpr int BUF_SIZE = 1024;

class FsFileWatcher : public Singleton<FsFileWatcher> {
public:
    int32_t GetNotifyId();
    bool InitNotify();
    int StartNotify(shared_ptr<WatcherInfo> info);
    int StopNotify(shared_ptr<WatcherInfo> info);
    void GetNotifyEvent();
    void AsyncGetNotifyEvent();
    bool AddWatcherInfo(shared_ptr<WatcherInfo> info);
    bool CheckEventValid(uint32_t event);

public:
    FsFileWatcher() = default;
    ~FsFileWatcher() = default;
    FsFileWatcher(const FsFileWatcher &) = delete;
    FsFileWatcher &operator=(const FsFileWatcher &) = delete;

private:
    uint32_t RemoveWatcherInfo(shared_ptr<WatcherInfo> info);
    void NotifyEvent(const struct inotify_event *event);
    int CloseNotifyFd();
    int CloseNotifyFdLocked();
    int NotifyToWatchNewEvents(const string &fileName, int wd, uint32_t watchEvents);
    void ReadNotifyEvent();
    void ReadNotifyEventLocked();
    void DestroyTaskThead();

private:
    mutex taskMutex_;
    mutex readMutex_;

    atomic<bool> taskRunning_ = false;
    thread taskThead_;

    bool run_ = false;
    bool reading_ = false;
    bool closed_ = false;
    int32_t notifyFd_ = -1;
    int32_t eventFd_ = -1;
    WatcherDataCache dataCache_;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_FILE_WATCHER_H
