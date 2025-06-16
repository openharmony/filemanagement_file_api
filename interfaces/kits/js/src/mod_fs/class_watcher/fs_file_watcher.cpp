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

int32_t FsFileWatcher::StartNotify(shared_ptr<WatcherInfo> info)
{
    if (notifyFd_ < 0) {
        HILOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    auto [isWatched, wd] = dataCache_.FindWatchedWd(info->fileName, info->events);
    if (isWatched && wd > 0) {
        info->wd = wd;
        return ERRNO_NOERR;
    }

    uint32_t watchEvents = 0;
    if (wd != -1) {
        watchEvents = dataCache_.GetFileEvents(info->fileName) | info->events;
    } else {
        watchEvents = info->events;
    }

    int32_t newWd = inotify_add_watch(notifyFd_, info->fileName.c_str(), watchEvents);
    if (newWd < 0) {
        HILOGE("Failed to start notify errCode:%{public}d", errno);
        return errno;
    }

    info->wd = newWd;
    dataCache_.UpdateWatchedEvents(info->fileName, newWd, watchEvents);
    return ERRNO_NOERR;
}

int32_t FsFileWatcher::NotifyToWatchNewEvents(const string &fileName, int32_t wd, uint32_t watchEvents)
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

    dataCache_.UpdateWatchedEvents(fileName, wd, watchEvents);
    return ERRNO_NOERR;
}

int32_t FsFileWatcher::CloseNotifyFd()
{
    int32_t closeRet = ERRNO_NOERR;

    if (!dataCache_.HasWatcherInfo()) {
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
    if (notifyFd_ < 0) {
        HILOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    uint32_t remainingEvents = RemoveWatcherInfo(info);
    if (remainingEvents > 0) {
        if (access(info->fileName.c_str(), F_OK) == 0) {
            return NotifyToWatchNewEvents(info->fileName, info->wd, remainingEvents);
        }
        HILOGE("The Watched file does not exist, and the remaining monitored events will be invalid.");
        return ERRNO_NOERR;
    }

    int32_t oldWd = -1;
    {
        lock_guard<mutex> lock(readMutex_);
        if (!(closed_ && reading_)) {
            oldWd = inotify_rm_watch(notifyFd_, info->wd);
        } else {
            HILOGE("rm watch fail");
        }
    }

    int32_t rmRet = 0;
    if (oldWd == -1) {
        rmRet = errno;
        HILOGE("Failed to stop notify errCode:%{public}d", rmRet);
    }

    dataCache_.RemoveFileWatcher(info->fileName);
    int32_t closeRet = CloseNotifyFdLocked();
    return rmRet != 0 ? rmRet : closeRet;
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
    struct pollfd fds[2] = { { 0 } };
    fds[0].fd = eventFd_;
    fds[0].events = POLLIN;
    fds[1].fd = notifyFd_;
    fds[1].events = POLLIN;
    int32_t ret = 0;

    while (run_) {
        ret = poll(fds, nfds, pollTimeoutMs);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                run_ = false;
                break;
            }
            if (static_cast<unsigned short>(fds[0].revents) & POLLIN) {
                run_ = false;
                break;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEventLocked();
            }
            continue;
        }
        if (ret == 0) {
            continue;
        }
        if (ret < 0 && errno == EINTR) {
            continue;
        }
        if (ret < 0 && errno != EINTR) {
            HILOGE("Failed to poll NotifyFd, errno=%{public}d", errno);
            break;
        }
    }
}

bool FsFileWatcher::AddWatcherInfo(shared_ptr<WatcherInfo> info)
{
    return dataCache_.AddWatcherInfo(info);
}

uint32_t FsFileWatcher::RemoveWatcherInfo(shared_ptr<WatcherInfo> info)
{
    return dataCache_.RemoveWatcherInfo(info);
}

void FsFileWatcher::NotifyEvent(const struct inotify_event *event)
{
    auto [matched, fileName, watcherInfos] = dataCache_.FindWatcherInfos(event->wd, event->mask);
    if (!matched) {
        HILOGE("Cannot find matched watcherInfos");
        return;
    }

    for (const auto &info : watcherInfos) {
        if (event->len > 0) {
            fileName += "/" + string(event->name);
        }
        info->TriggerCallback(fileName, event->mask & IN_ALL_EVENTS, event->cookie);
    }
}

bool FsFileWatcher::CheckEventValid(uint32_t event)
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
        if (taskThead_.get_id() != std::this_thread::get_id()) {
            taskThead_.join();
        } else {
            taskThead_.detach();
        }
    }

    {
        lock_guard<mutex> lock(taskMutex_);
        if (taskRunning_) {
            taskRunning_ = false;
            run_ = false;
        }
    }
    dataCache_.ClearCache();
}
} // namespace OHOS::FileManagement::ModuleFileIO
