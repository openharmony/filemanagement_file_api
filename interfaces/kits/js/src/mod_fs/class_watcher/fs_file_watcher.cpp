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
#include "fs_file_watcher.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <poll.h>
#include <unistd.h>

#include <sys/eventfd.h>

#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

mutex FsFileWatcher::watchMutex_;

FsFileWatcher::FsFileWatcher() {}

FsFileWatcher::~FsFileWatcher() {}

int32_t FsFileWatcher::GetNotifyId()
{
    return notifyFd_;
}

bool FsFileWatcher::InitNotify()
{
    notifyFd_ = inotify_init();
    if (notifyFd_ < 0) {
        HILOGE("Failed to init notify errCode:%{public}d", errno);
        return false;
    }
    eventFd_ = eventfd(0, EFD_CLOEXEC);
    if (eventFd_ < 0) {
        HILOGE("Failed to init eventfd errCode:%{public}d", errno);
        return false;
    }
    return true;
}

tuple<bool, int32_t> FsFileWatcher::CheckEventWatched(const string &fileName, const uint32_t &event)
{
    int32_t wd = -1;
    auto iter = wdFileNameMap_.find(fileName);
    if (iter == wdFileNameMap_.end()) {
        return { false, wd };
    }
    wd = iter->second.first;
    if ((iter->second.second & event) == event) {
        return { true, wd };
    }
    return { false, wd };
}

int32_t FsFileWatcher::StartNotify(shared_ptr<WatcherInfo> info)
{
    lock_guard<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        HILOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    auto [isWatched, wd] = CheckEventWatched(info->fileName, info->events);
    if (isWatched && wd > 0) {
        info->wd = wd;
        return ERRNO_NOERR;
    }
    uint32_t watchEvents = 0;
    if (wd != -1) {
        watchEvents = wdFileNameMap_[info->fileName].second | info->events;
    } else {
        watchEvents = info->events;
    }
    int32_t newWd = inotify_add_watch(notifyFd_, info->fileName.c_str(), watchEvents);
    if (newWd < 0) {
        HILOGE("Failed to start notify errCode:%{public}d", errno);
        return errno;
    }
    info->wd = newWd;
    wdFileNameMap_[info->fileName].first = newWd;
    wdFileNameMap_[info->fileName].second = watchEvents;
    return ERRNO_NOERR;
}

int32_t FsFileWatcher::NotifyToWatchNewEvents(const string &fileName, const int32_t &wd, const uint32_t &watchEvents)
{
    int32_t newWd = inotify_add_watch(notifyFd_, fileName.c_str(), watchEvents);
    if (newWd < 0) {
        HILOGE("Failed to start new notify errCode:%{public}d", errno);
        return errno;
    }

    if (newWd != wd) {
        HILOGE("New notify wd is error");
        return EIO;
    }
    wdFileNameMap_[fileName].second = watchEvents;
    return ERRNO_NOERR;
}

int32_t FsFileWatcher::CloseNotifyFd()
{
    int32_t closeRet = ERRNO_NOERR;

    if (watcherInfoSet_.size() == 0) {
        run_ = false;
        closeRet = close(notifyFd_);
        if (closeRet != 0) {
            HILOGE("Failed to stop notify close fd errCode:%{public}d", errno);
        }
        notifyFd_ = -1;
        closeRet = close(eventFd_);
        if (closeRet != 0) {
            HILOGE("Failed to close eventfd errCode:%{public}d", errno);
        }
        eventFd_ = -1;
        DestroyTaskThead();
    }

    closed_ = false;
    return closeRet;
}

int FsFileWatcher::CloseNotifyFdLocked()
{
    {
        lock_guard<mutex> lock(readMutex_);
        closed_ = true;
        if (reading_) {
            HILOGE("close while reading");
            return ERRNO_NOERR;
        }
    }
    return CloseNotifyFd();
}

int32_t FsFileWatcher::StopNotify(shared_ptr<WatcherInfo> info)
{
    unique_lock<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        HILOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }
    uint32_t newEvents = RemoveWatcherInfo(info);
    if (newEvents > 0) {
        if (access(info->fileName.c_str(), F_OK) == 0) {
            return NotifyToWatchNewEvents(info->fileName, info->wd, newEvents);
        }
        HILOGE("The Watched file does not exist, and the remaining monitored events will be invalid.");
        return ERRNO_NOERR;
    }
    int oldWd = -1;
    {
        lock_guard<mutex> lock(readMutex_);
        if (!(closed_ && reading_)) {
            oldWd = inotify_rm_watch(notifyFd_, info->wd);
        } else {
            HILOGE("rm watch fail");
        }
    }
    if (oldWd == -1) {
        int rmErr = errno;
        if (access(info->fileName.c_str(), F_OK) == 0) {
            HILOGE("Failed to stop notify errCode:%{public}d", rmErr);
            wdFileNameMap_.erase(info->fileName);
            CloseNotifyFdLocked();
            return rmErr;
        }
    }
    wdFileNameMap_.erase(info->fileName);
    return CloseNotifyFdLocked();
}

void FsFileWatcher::ReadNotifyEvent()
{
    int32_t len = 0;
    int32_t index = 0;
    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *event = nullptr;
    do {
        len = read(notifyFd_, &buf, sizeof(buf));
        if (len < 0 && errno != EINTR) {
            HILOGE("Read notify event failed! ret: %d", errno);
            break;
        }
    } while (len < 0);
    while (index < len) {
        event = reinterpret_cast<inotify_event *>(buf + index);
        NotifyEvent(event);
        index += sizeof(struct inotify_event) + static_cast<int32_t>(event->len);
    }
}

void FsFileWatcher::ReadNotifyEventLocked()
{
    {
        lock_guard<mutex> lock(readMutex_);
        if (closed_) {
            HILOGE("read after close");
            return;
        }
        reading_ = true;
    }
    ReadNotifyEvent();
    {
        lock_guard<mutex> lock(readMutex_);
        reading_ = false;
        if (closed_) {
            HILOGE("close after read");
            CloseNotifyFd();
            return;
        }
    }
}

void FsFileWatcher::AsyncGetNotifyEvent()
{
    lock_guard<mutex> lock(taskMutex_);
    if (!taskRunning_) {
        taskRunning_ = true;
        taskThead_ = thread(&FsFileWatcher::GetNotifyEvent, this);
    }
}

void FsFileWatcher::GetNotifyEvent()
{
    if (run_) {
        return;
    }
    run_ = true;
    nfds_t nfds = 2;
    struct pollfd fds[2];
    fds[0].fd = eventFd_;
    fds[0].events = 0;
    fds[1].fd = notifyFd_;
    fds[1].events = POLLIN;
    int32_t ret = 0;
    while (run_) {
        ret = poll(fds, nfds, -1);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                run_ = false;
                return;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEventLocked();
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            HILOGE("Failed to poll NotifyFd, errno=%{public}d", errno);
            return;
        }
    }
}

bool FsFileWatcher::AddWatcherInfo(const string &fileName, shared_ptr<WatcherInfo> info)
{
    for (auto &iter : watcherInfoSet_) {
        if (iter->fileName == info->fileName && iter->events == info->events) {
            bool isSame = iter->callback->IsStrictEquals(info->callback);
            if (isSame) {
                HILOGE("Faile to add watcher, fileName:%{public}s the callback is same", fileName.c_str());
                return false;
            }
        }
    }
    watcherInfoSet_.insert(info);
    return true;
}

uint32_t FsFileWatcher::RemoveWatcherInfo(shared_ptr<WatcherInfo> info)
{
    watcherInfoSet_.erase(info);
    uint32_t otherEvents = 0;
    for (const auto &iter : watcherInfoSet_) {
        if (iter->fileName == info->fileName && iter->wd > 0) {
            otherEvents |= iter->events;
        }
    }
    return otherEvents;
}

static bool CheckIncludeEvent(const uint32_t &mask, const uint32_t &event)
{
    if ((mask & event) > 0) {
        return true;
    }
    return false;
}

void FsFileWatcher::NotifyEvent(const struct inotify_event *event)
{
    lock_guard<mutex> lock(watchMutex_);
    string tempFileName;
    auto found = find_if(wdFileNameMap_.begin(), wdFileNameMap_.end(),
        [event](const pair<string, pair<int32_t, uint32_t>> &iter) { return iter.second.first == event->wd; });
    if (found != wdFileNameMap_.end()) {
        tempFileName = found->first;
    }

    for (const auto &iter : watcherInfoSet_) {
        string fileName = tempFileName;
        uint32_t watchEvent = 0;
        if ((iter->fileName == fileName) && (iter->wd > 0)) {
            watchEvent = iter->events;
        }
        if (!CheckIncludeEvent(event->mask, watchEvent)) {
            continue;
        }
        if (event->len > 0) {
            fileName += "/" + string(event->name);
        }
        iter->TriggerCallback(fileName, event->mask & IN_ALL_EVENTS, event->cookie);
    }
}

bool FsFileWatcher::CheckEventValid(const uint32_t &event)
{
    if ((event & IN_ALL_EVENTS) == event) {
        return true;
    } else {
        HILOGE("Param event:%{public}x is not valid", event);
        return false;
    }
}

void FsFileWatcher::DestroyTaskThead()
{
    if (taskThead_.joinable()) {
        taskThead_.join();
    }

    lock_guard<mutex> lock(taskMutex_);
    if (taskRunning_) {
        taskRunning_ = false;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO
