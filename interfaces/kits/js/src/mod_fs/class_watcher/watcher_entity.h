/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_WATCHER_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_WATCHER_ENTITY_H

#include <memory>
#include <mutex>
#include <string>
#include <sys/inotify.h>
#include <unordered_map>
#include <unordered_set>
#include <uv.h>

#include "filemgmt_libn.h"
#include "singleton.h"
namespace OHOS::FileManagement::ModuleFileIO {
using WatcherCallback = void (*)(napi_env env,
                                 LibN::NRef &callback,
                                 const std::string &filename,
                                 const uint32_t &event,
                                 const uint32_t &cookie);

constexpr int BUF_SIZE = 1024;
struct WatcherInfoArg {
    std::string fileName = "";
    uint32_t events = 0;
    int wd = -1;
    napi_env env = nullptr;
    LibN::NRef nRef;
    explicit WatcherInfoArg(LibN::NVal jsVal) : nRef(jsVal) {}
    ~WatcherInfoArg() = default;
};

class FileWatcher : public Singleton<FileWatcher> {
public:
    FileWatcher();
    ~FileWatcher();

    FileWatcher(FileWatcher const &) = delete;
    void operator=(FileWatcher const &) = delete;

    int32_t GetNotifyId();
    bool InitNotify();
    int StartNotify(std::shared_ptr<WatcherInfoArg> arg);
    int StopNotify(std::shared_ptr<WatcherInfoArg> arg);
    void GetNotifyEvent(WatcherCallback callback);
    void ReadNotifyEvent(WatcherCallback callback);
    bool AddWatcherInfo(const std::string &fileName, std::shared_ptr<WatcherInfoArg> arg);
    bool CheckEventValid(const uint32_t &event);
private:
    uint32_t RemoveWatcherInfo(std::shared_ptr<WatcherInfoArg> arg);
    std::tuple<bool, int> CheckEventWatched(const std::string &fileName, const uint32_t &event);
    void NotifyEvent(const struct inotify_event *event, WatcherCallback callback);
    int CloseNotifyFd();
    int NotifyToWatchNewEvents(const std::string &fileName, const int &wd, const uint32_t &watchEvents);

private:
    static std::mutex watchMutex_;
    bool run_ = false;
    int32_t notifyFd_ = -1;
    int32_t eventFd_ = -1;
    std::unordered_set<std::shared_ptr<WatcherInfoArg>> watcherInfoSet_;
    std::unordered_map<std::string, std::pair<int, uint32_t>> wdFileNameMap_;
};

struct WatcherEntity {
    std::shared_ptr<WatcherInfoArg> data_;
};
} // namespace OHOS::FileManagement::ModuleFileIO namespace OHOS
#endif
