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
#include "watcher_entity.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
using namespace std;

mutex FileWatcher::watchMutex_;

FileWatcher::FileWatcher() {}

FileWatcher::~FileWatcher() {}

int32_t FileWatcher::GetNotifyId()
{
    return notifyFd_;
}

bool FileWatcher::InitNotify()
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

tuple<bool, int> FileWatcher::CheckEventWatched(const string &fileName, const uint32_t &event)
{
    int wd = -1;
    auto iter = wdFileNameMap_.find(fileName);
    if (iter == wdFileNameMap_.end()) {
        return {false, wd};
    }
    wd = iter->second.first;
    if ((iter->second.second & event) == event) {
        return {true, wd};
    }
    return {false, wd};
}

int FileWatcher::StartNotify(shared_ptr<WatcherInfoArg> arg)
{
    lock_guard<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        HILOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    auto [isWatched, wd] = CheckEventWatched(arg->fileName, arg->events);
    if (isWatched && wd > 0) {
        arg->wd = wd;
        return ERRNO_NOERR;
    }
    uint32_t watchEvents = 0;
    if (wd != -1) {
        watchEvents = wdFileNameMap_[arg->fileName].second | arg->events;
    } else {
        watchEvents = arg->events;
    }
    int newWd = inotify_add_watch(notifyFd_, arg->fileName.c_str(), watchEvents);
    if (newWd < 0) {
        HILOGE("Failed to start notify errCode:%{public}d", errno);
        return errno;
    }
    arg->wd = newWd;
    wdFileNameMap_[arg->fileName].first = newWd;
    wdFileNameMap_[arg->fileName].second = watchEvents;
    return ERRNO_NOERR;
}

int FileWatcher::NotifyToWatchNewEvents(const string &fileName, const int &wd, const uint32_t &watchEvents)
{
    int newWd = inotify_add_watch(notifyFd_, fileName.c_str(), watchEvents);
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

int FileWatcher::CloseNotifyFd()
{
    int closeRet = ERRNO_NOERR;
    if (watcherInfoSet_.size() == 0) {
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
        run_ = false;
    }

    return closeRet;
}

int FileWatcher::StopNotify(shared_ptr<WatcherInfoArg> arg)
{
    unique_lock<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        HILOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }
    uint32_t newEvents = RemoveWatcherInfo(arg);
    if (newEvents > 0) {
        if (access(arg->fileName.c_str(), F_OK) == 0) {
            return NotifyToWatchNewEvents(arg->fileName, arg->wd, newEvents);
        }
        HILOGE("The Watched file does not exist, and the remaining monitored events will be invalid.");
        return ERRNO_NOERR;
    }
    if (inotify_rm_watch(notifyFd_, arg->wd) == -1) {
        int rmErr = errno;
        if (access(arg->fileName.c_str(), F_OK) == 0) {
            HILOGE("Failed to stop notify errCode:%{public}d", rmErr);
            wdFileNameMap_.erase(arg->fileName);
            CloseNotifyFd();
            return rmErr;
        }
    }
    wdFileNameMap_.erase(arg->fileName);
    return CloseNotifyFd();
}

void FileWatcher::ReadNotifyEvent(WatcherCallback callback)
{
    int len = 0;
    int index = 0;
    char buf[BUF_SIZE] = {0};
    struct inotify_event *event = nullptr;
    while (((len = read(notifyFd_, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {};
    while (index < len) {
        event = reinterpret_cast<inotify_event *>(buf + index);
        NotifyEvent(event, callback);
        index += sizeof(struct inotify_event) + static_cast<int>(event->len);
    }
}

void FileWatcher::GetNotifyEvent(WatcherCallback callback)
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
    while (run_) {
        int ret = poll(fds, nfds, -1);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                run_ = false;
                return;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEvent(callback);
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            HILOGE("Failed to poll NotifyFd, errno=%{public}d", errno);
            return;
        }
    }
}

bool FileWatcher::AddWatcherInfo(const string &fileName, shared_ptr<WatcherInfoArg> arg)
{
    for (auto &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->events == arg->events) {
            bool isSame = false;
            napi_strict_equals(iter->env, iter->nRef.Deref(iter->env).val_, arg->nRef.Deref(arg->env).val_, &isSame);
            if (isSame) {
                HILOGE("Faile to add watcher, fileName:%{public}s the callback is same", fileName.c_str());
                return false;
            }
        }
    }
    watcherInfoSet_.insert(arg);
    return true;
}

uint32_t FileWatcher::RemoveWatcherInfo(shared_ptr<WatcherInfoArg> arg)
{
    watcherInfoSet_.erase(arg);
    uint32_t otherEvents = 0;
    for (const auto &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->wd > 0) {
            otherEvents |= iter->events;
        }
    }
    return otherEvents;
}

bool CheckIncludeEvent(const uint32_t &mask, const uint32_t &event)
{
    if ((mask & event) > 0) {
        return true;
    }
    return false;
}

void FileWatcher::NotifyEvent(const struct inotify_event *event, WatcherCallback callback)
{
    lock_guard<mutex> lock(watchMutex_);
    string tempFileName;
    auto found = find_if(wdFileNameMap_.begin(), wdFileNameMap_.end(),
        [event](const pair<std::string, std::pair<int, uint32_t>> &iter) {
            return iter.second.first == event->wd;
    });
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
        callback(iter->env, iter->nRef, fileName, event->mask & IN_ALL_EVENTS, event->cookie);
    }
}

bool FileWatcher::CheckEventValid(const uint32_t &event)
{
    if ((event & IN_ALL_EVENTS) == event) {
        return true;
    } else {
        HILOGE("Param event:%{public}x is not valid", event);
        return false;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO
