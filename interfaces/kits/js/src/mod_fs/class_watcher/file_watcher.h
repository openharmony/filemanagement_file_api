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

#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H
#include <memory>
#include <sys/inotify.h>
#include <string>
#include "watcher_entity.h"
namespace OHOS::FileManagement::ModuleFileIO {
using WatcherCallback = void (*)(napi_env env, napi_ref callback, const std::string &filename, const uint32_t &event);
struct ErrorInfo {
    int errCode;
    std::string errMsg;
};

class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();
    bool InitNotify(int &fd);
    bool StartNotify(std::shared_ptr<WatcherInfoArg> &arg);
    bool StopNotify(std::shared_ptr<WatcherInfoArg> &arg);
    void GetNotifyEvent(std::shared_ptr<WatcherInfoArg> &arg, WatcherCallback callback);

private:
    void HandleEvent(std::shared_ptr<WatcherInfoArg> &arg, const struct inotify_event *event, WatcherCallback callback);
    bool run_;
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif
