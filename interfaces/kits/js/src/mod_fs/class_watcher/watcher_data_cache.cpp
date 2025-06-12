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
#include "watcher_data_cache.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {

bool WatcherDataCache::AddWatcherInfo(std::shared_ptr<WatcherInfo> info)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (auto &iter : watcherInfoCache_) {
        if (iter->fileName == info->fileName && iter->events == info->events) {
            bool isSame = iter->callback->IsStrictEquals(info->callback);
            if (isSame) {
                HILOGE("Failed to add watcher, fileName:%{private}s the callback is same", info->fileName.c_str());
                return false;
            }
        }
    }
    watcherInfoCache_.push_back(info);
    wdFileNameCache_[info->fileName] = std::make_pair(info->wd, info->events);
    return true;
}

uint32_t WatcherDataCache::RemoveWatcherInfo(std::shared_ptr<WatcherInfo> info)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = std::find(watcherInfoCache_.begin(), watcherInfoCache_.end(), info);
    if (it != watcherInfoCache_.end()) {
        watcherInfoCache_.erase(it);
    }

    uint32_t remainingEvents = 0;
    for (const auto &iter : watcherInfoCache_) {
        if (iter->fileName == info->fileName && iter->wd > 0) {
            remainingEvents |= iter->events;
        }
    }

    return remainingEvents;
}

bool WatcherDataCache::RemoveFileWatcher(const std::string &fileName)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);

    auto iter = wdFileNameCache_.find(fileName);
    if (iter == wdFileNameCache_.end()) {
        return false;
    }
    wdFileNameCache_.erase(iter);

    watcherInfoCache_.erase(std::remove_if(watcherInfoCache_.begin(), watcherInfoCache_.end(),
                                [&fileName](const std::shared_ptr<WatcherInfo> &info) {
                                    return info->fileName == fileName;
                                }),
        watcherInfoCache_.end());

    return true;
}

std::tuple<bool, int32_t> WatcherDataCache::FindWatchedWd(const std::string &fileName, uint32_t event)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);

    int32_t wd = -1;
    auto iter = wdFileNameCache_.find(fileName);
    if (iter == wdFileNameCache_.end()) {
        return { false, wd };
    }

    wd = iter->second.first;
    if ((iter->second.second & event) == event) {
        return { true, wd };
    }

    return { false, wd };
}

bool WatcherDataCache::UpdateWatchedEvents(const std::string &fileName, int32_t wd, uint32_t events)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto iter = wdFileNameCache_.find(fileName);
    if (iter == wdFileNameCache_.end()) {
        return false;
    }

    iter->second = std::make_pair(wd, events);
    return true;
}

static bool CheckIncludeEvent(uint32_t mask, uint32_t event)
{
    return (mask & event) > 0;
}

std::tuple<bool, std::string, std::vector<std::shared_ptr<WatcherInfo>>> WatcherDataCache::FindWatcherInfos(
    int32_t wd, uint32_t eventMask)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    std::string fileName;
    bool found = false;
    for (const auto &[key, val] : wdFileNameCache_) {
        if (val.first == wd) {
            fileName = key;
            found = true;
            break;
        }
    }

    if (!found) {
        return { false, "", {} };
    }

    std::vector<std::shared_ptr<WatcherInfo>> matchedInfos;
    for (const auto &info : watcherInfoCache_) {
        uint32_t watchEvent = 0;
        if ((info->fileName == fileName) && (info->wd > 0)) {
            watchEvent = info->events;
        }
        if (CheckIncludeEvent(eventMask, watchEvent)) {
            matchedInfos.push_back(info);
        }
    }
    return { !matchedInfos.empty(), fileName, matchedInfos };
}

uint32_t WatcherDataCache::GetFileEvents(const std::string &fileName)
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto iter = wdFileNameCache_.find(fileName);
    if (iter == wdFileNameCache_.end()) {
        return 0;
    }
    return iter->second.second;
}

bool WatcherDataCache::HasWatcherInfo() const
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return !watcherInfoCache_.empty();
}

void WatcherDataCache::ClearCache()
{
    std::lock_guard<std::mutex> lock(cacheMutex_);
    watcherInfoCache_.clear();
    wdFileNameCache_.clear();
}

} // namespace OHOS::FileManagement::ModuleFileIO