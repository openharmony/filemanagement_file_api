/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "watcher_impl.h"
#include <unistd.h>

using namespace OHOS::CJSystemapi::FileFs;

namespace OHOS::CJSystemapi {
using namespace std;

mutex WatcherImpl::watchMutex_;

WatcherImpl::WatcherImpl() {}

int32_t WatcherImpl::GetNotifyId()
{
    return notifyFd_;
}

bool WatcherImpl::InitNotify()
{
    notifyFd_ = inotify_init();
    if (notifyFd_ < 0) {
        LOGE("Failed to init notify errCode:%{public}d", errno);
        return false;
    }
    return true;
}

tuple<bool, int> WatcherImpl::CheckEventWatched(const string &fileName, const uint32_t &event)
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

bool WatcherImpl::AddWatcherInfo(const string &fileName, shared_ptr<WatcherInfoArg> arg)
{
    for (auto const &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->events == arg->events) {
            LOGE("Faile to add watcher, fileName:%{public}s the callback is same", fileName.c_str());
            return false;
        }
    }
    watcherInfoSet_.insert(arg);
    return true;
}

bool CheckIncludeEvent(const uint32_t &mask, const uint32_t &event)
{
    if ((mask & event) > 0) {
        return true;
    }
    return false;
}

uint32_t WatcherImpl::RemoveWatcherInfo(shared_ptr<WatcherInfoArg> arg)
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

int32_t WatcherImpl::StartNotify()
{
    lock_guard<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        LOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return GetErrorCode(EIO);
    }
    auto [isWatched, wd] = CheckEventWatched(data_->fileName, data_->events);
    if (isWatched && wd > 0) {
        data_->wd = wd;
        return SUCCESS_CODE;
    }
    uint32_t watchEvents = 0;
    if (wd != -1) {
        watchEvents = wdFileNameMap_[data_->fileName].second | data_->events;
    } else {
        watchEvents = data_->events;
    }
    int newWd = inotify_add_watch(notifyFd_, data_->fileName.c_str(), watchEvents);
    if (newWd < 0) {
        LOGE("Failed to start notify errCode:%{public}d", errno);
        return GetErrorCode(errno);
    }
    data_->wd = newWd;
    wdFileNameMap_[data_->fileName].first = newWd;
    wdFileNameMap_[data_->fileName].second = watchEvents;
    return SUCCESS_CODE;
}

int WatcherImpl::NotifyToWatchNewEvents(const string &fileName, const int &wd, const uint32_t &watchEvents)
{
    int newWd = inotify_add_watch(notifyFd_, fileName.c_str(), watchEvents);
    if (newWd < 0) {
        LOGE("Failed to start new notify errCode:%{public}d", errno);
        return GetErrorCode(errno);
    }

    if (newWd != wd) {
        LOGE("New notify wd is error");
        return GetErrorCode(EIO);
    }
    wdFileNameMap_[fileName].second = watchEvents;
    return SUCCESS_CODE;
}

void WatcherImpl::GetNotifyEvent()
{
    if (run_) {
        return;
    }
    run_ = true;
    char buf[BUF_SIZE] = {0};
    struct inotify_event *event = nullptr;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(notifyFd_, &fds);
    while (run_) {
        if (notifyFd_ < 0) {
            LOGE("Failed to run Listener Thread because notifyFd_:%{public}d", notifyFd_);
            break;
        }
        if (select(notifyFd_ + 1, &fds, nullptr, nullptr, nullptr) > 0) {
            int len = 0;
            int index = 0;
            while (((len = read(notifyFd_, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {};
            while (index < len) {
                event = reinterpret_cast<inotify_event *>(buf + index);
                NotifyEvent(event);
                index += static_cast<int32_t>(sizeof(struct inotify_event) + event->len);
            }
        }
    }
}

void WatcherImpl::NotifyEvent(const struct inotify_event *event)
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
        CWatchEvent ret = {
            .fileName = fileName.c_str(),
            .event = event->mask & IN_ALL_EVENTS,
            .cookie = event->cookie };
        data_->watchCallback_(ret);
    }
}

int WatcherImpl::CloseNotifyFd()
{
    int closeRet = SUCCESS_CODE;
    if (watcherInfoSet_.size() == 0) {
        closeRet = close(notifyFd_);
        if (closeRet != 0) {
            LOGE("Failed to stop notify close fd errCode:%{public}d", closeRet);
        }
        notifyFd_ = -1;
        run_ = false;
    }

    return closeRet;
}

int32_t WatcherImpl::StopNotify()
{
    unique_lock<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        LOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
        return EIO;
    }
    uint32_t newEvents = RemoveWatcherInfo(data_);
    if (newEvents > 0) {
        return NotifyToWatchNewEvents(data_->fileName, data_->wd, newEvents);
    }
    if (inotify_rm_watch(notifyFd_, data_->wd) == -1) {
        int rmErr = errno;
        if (access(data_->fileName.c_str(), F_OK) == 0) {
            LOGE("Failed to stop notify errCode:%{public}d", rmErr);
            wdFileNameMap_.erase(data_->fileName);
            CloseNotifyFd();
            return rmErr;
        }
    }
    wdFileNameMap_.erase(data_->fileName);
    return CloseNotifyFd();
}

bool WatcherImpl::CheckEventValid(const uint32_t &event)
{
    if ((event & IN_ALL_EVENTS) == event) {
        return true;
    } else {
        LOGE("Param event:%{public}x is not valid", event);
        return false;
    }
}

}