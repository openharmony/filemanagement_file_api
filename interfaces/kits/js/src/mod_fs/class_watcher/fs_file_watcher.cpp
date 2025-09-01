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

bool FsFileWatcher::TryInitNotify()
{
    lock_guard<mutex> lock(notifyMutex_);
    return notifyFd_ >= 0 || InitNotify();
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
    if (!info) {
        HILOGE("Invalid param: info");
        return EINVAL;
    }

    if (notifyFd_ < 0) {
        HILOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    auto [isWatched, wd, events] = dataCache_.FindWatchedEvents(info->fileName, info->events);
    if (isWatched && wd > 0) {
        info->wd = wd;
        return ERRNO_NOERR;
    }

    uint32_t watchEvents = 0;
    if (wd != -1) {
        watchEvents = events | info->events;
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
        WakeupThread();
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
        HILOGD("NotifyFd and eventFd have been closed");
    }

    closed_ = false;
    return closeRet;
}

int32_t FsFileWatcher::CloseNotifyFdLocked()
{
    scoped_lock lock(readMutex_, notifyMutex_);
    closed_ = true;
    if (reading_) {
        HILOGE("Close while reading");
        closed_ = false;
        return ERRNO_NOERR;
    }
    return CloseNotifyFd();
}

int32_t FsFileWatcher::StopNotify(shared_ptr<WatcherInfo> info)
{
    if (!info) {
        HILOGE("Invalid param: info");
        return EINVAL;
    }

    if (notifyFd_ < 0) {
        HILOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }

    uint32_t remainingEvents = RemoveWatcherInfo(info);
    if (remainingEvents > 0) {
        // There are still events remaining to be listened for.
        if (access(info->fileName.c_str(), F_OK) == 0) {
            return NotifyToWatchNewEvents(info->fileName, info->wd, remainingEvents);
        }
        HILOGE("The Watched file does not exist, and the remaining monitored events will be invalid.");
        return ERRNO_NOERR;
    }

    // No events remain to be listened for, and proceed to the file watch removal process.
    int32_t oldWd = -1;
    {
        lock_guard<mutex> lock(readMutex_);
        if (!(closed_ && reading_)) {
            oldWd = inotify_rm_watch(notifyFd_, info->wd);
        } else {
            HILOGE("Rm watch fail");
        }
    }

    if (oldWd == -1) {
        int32_t rmErr = errno;
        if (access(info->fileName.c_str(), F_OK) == 0) {
            HILOGE("Failed to stop notify errCode:%{public}d", rmErr);
            dataCache_.RemoveWatchedEvents(info->fileName);
            CloseNotifyFdLocked();
            return rmErr;
        }
    }

    dataCache_.RemoveWatchedEvents(info->fileName);
    auto ret = CloseNotifyFdLocked();
    HILOGD("Close notifyFd and eventFd ret: %{public}d", ret);
    return ret;
}

void FsFileWatcher::ReadNotifyEvent()
{
    int32_t len = 0;
    uint32_t index = 0;
    char buf[BUF_SIZE] = { 0 };
    struct inotify_event *event = nullptr;
    uint32_t eventSize = static_cast<uint32_t>(sizeof(struct inotify_event));

    do {
        len = read(notifyFd_, &buf, sizeof(buf));
        if (len < 0 && errno != EINTR) {
            HILOGE("Read notify event failed! ret: %d", errno);
            break;
        }
    } while (len < 0);

    while (len > 0 && index < len) {
        event = reinterpret_cast<inotify_event *>(buf + index);

        // Incomplete data: remaining bytes less than event struct size
        if ((len - index) < eventSize) {
            HILOGE(
                "Out of bounds access, len:%{public}d, index: %{public}u, inotify: %{public}u", len, index, eventSize);
            break;
        }

        // Incomplete data: remaining bytes less than (event struct size + event->len)
        if (event->len > len - index - eventSize) {
            HILOGE("Out of bounds access, index: %{public}u, inotify: %{public}u, event :%{public}u, len: %{public}d",
                index, eventSize, event->len, len);
            break;
        }

        NotifyEvent(event);
        index += eventSize + event->len;
    }
}

void FsFileWatcher::ReadNotifyEventLocked()
{
    {
        lock_guard<mutex> lock(readMutex_);
        if (closed_) {
            HILOGE("Read after close");
            return;
        }
        reading_ = true;
    }
    ReadNotifyEvent();
    {
        scoped_lock lock(readMutex_, notifyMutex_);
        reading_ = false;
        if (closed_) {
            HILOGE("Close after read");
            CloseNotifyFd();
            return;
        }
    }
}

void FsFileWatcher::AsyncGetNotifyEvent()
{
    bool expected = false;
    if (taskRunning_.compare_exchange_strong(expected, true)) {
        taskThread_ = thread(&FsFileWatcher::GetNotifyEvent, this);
    }
}

void FsFileWatcher::GetNotifyEvent()
{
    if (run_) {
        return;
    }

    run_ = true;
    nfds_t nfds = 2;
    struct pollfd fds[2] = { { -1 } };
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
        } else if (ret < 0 && errno != EINTR) {
            HILOGE("Failed to poll NotifyFd, errno=%{public}d", errno);
            break;
        }
        // Ignore cases where poll returns 0 (timeout) or EINTR (interrupted system call)
    }
    DestroyTaskThread();
    HILOGD("The task has been completed.");
}

bool FsFileWatcher::AddWatcherInfo(shared_ptr<WatcherInfo> info)
{
    if (!info) {
        HILOGE("Invalid param: info");
        return false;
    }
    return dataCache_.AddWatcherInfo(info);
}

uint32_t FsFileWatcher::RemoveWatcherInfo(shared_ptr<WatcherInfo> info)
{
    if (!info) {
        HILOGE("Invalid param: info");
        return EINVAL;
    }
    return dataCache_.RemoveWatcherInfo(info);
}

void FsFileWatcher::NotifyEvent(const struct inotify_event *event)
{
    if (!event) {
        HILOGE("Invalid inotify event");
        return;
    }

    auto [matched, fileName, watcherInfos] = dataCache_.FindWatcherInfos(event->wd, event->mask);
    if (!matched) {
        // ignore unmatched event
        HILOGD("Cannot find matched watcherInfos");
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

void FsFileWatcher::DestroyTaskThread()
{
    bool expected = true;
    if (taskRunning_.compare_exchange_strong(expected, false)) {
        run_ = false;
        dataCache_.ClearCache();

        if (taskThread_.joinable()) {
            taskThread_.detach();
        }
    }
}

void FsFileWatcher::WakeupThread()
{
    if (taskRunning_ && eventFd_ >= 0 && taskThread_.joinable()) {
        ssize_t ret = write(eventFd_, &wakeupSignal, sizeof(wakeupSignal));
        if (ret != sizeof(wakeupSignal)) {
            HILOGE("WakeupThread failed! write ret: %{public}zd, errno: %{public}d", ret, errno);
        }
    } else {
        HILOGE("Cannot wakeup thread! taskRunning: %{public}d, eventFd_: %{public}d, taskThread joinable: %{public}d",
            static_cast<int>(taskRunning_), eventFd_, static_cast<int>(taskThread_.joinable()));
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO
