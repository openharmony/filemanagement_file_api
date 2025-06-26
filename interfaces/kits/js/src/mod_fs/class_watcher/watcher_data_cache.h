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
#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_WATCHER_DATA_CACHE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_WATCHER_DATA_CACHE_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "fs_watch_entity.h"

namespace OHOS::FileManagement::ModuleFileIO {

class WatcherDataCache {
public:
    bool AddWatcherInfo(std::shared_ptr<WatcherInfo> info);
    uint32_t RemoveWatcherInfo(std::shared_ptr<WatcherInfo> info);
    bool RemoveFileWatcher(const std::string &fileName);
    std::tuple<bool, int32_t> FindWatchedWd(const std::string &fileName, uint32_t event);
    bool UpdateWatchedEvents(const std::string &fileName, int32_t wd, uint32_t events);
    std::tuple<bool, std::string, std::vector<std::shared_ptr<WatcherInfo>>> FindWatcherInfos(
        int32_t wd, uint32_t eventMask);
    uint32_t GetFileEvents(const std::string &fileName);
    bool HasWatcherInfo() const;
    void ClearCache();

public:
    WatcherDataCache() = default;
    ~WatcherDataCache() = default;
    WatcherDataCache(const WatcherDataCache &) = delete;
    WatcherDataCache &operator=(const WatcherDataCache &) = delete;

private:
    mutable std::mutex cacheMutex_;
    std::vector<std::shared_ptr<WatcherInfo>> watcherInfoCache_;
    std::unordered_map<std::string, std::pair<int32_t, uint32_t>> wdFileNameCache_;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_WATCHER_DATA_CACHE_H