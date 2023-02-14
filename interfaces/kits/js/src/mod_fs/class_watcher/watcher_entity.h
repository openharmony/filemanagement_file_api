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
#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_ENTITY_H
#include <memory>
#include <string>
#include <sys/inotify.h>
#include <vector>

#include "filemgmt_libn.h"
namespace OHOS::FileManagement::ModuleFileIO {
using WatcherCallback = void (*)(napi_env env, LibN::NRef &callback,
    const std::string &filename, const uint32_t &event);
constexpr int BUF_SIZE = 1024;
struct WatcherInfoArg {
    std::string filename = "";
    uint32_t events = 0;
    int fd = -1;
    int wd = -1;
    napi_env env = nullptr;
    LibN::NRef nRef;
    explicit WatcherInfoArg(LibN::NVal jsVal) : nRef(jsVal) {}
    ~WatcherInfoArg() = default;
};

class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();
    bool InitNotify(int &fd);
    bool StartNotify(WatcherInfoArg &arg);
    bool StopNotify(const WatcherInfoArg &arg);
    void GetNotifyEvent(WatcherInfoArg &arg, WatcherCallback callback);

private:
    void HandleEvent(WatcherInfoArg &arg, const struct inotify_event *event,
                     WatcherCallback callback);
    bool run_ = false;
};

struct WatcherEntity {
    std::unique_ptr<WatcherInfoArg> data_;
    std::unique_ptr<FileWatcher> watcherPtr_;
};
} // namespace OHOS::FileManagement::ModuleFileIO namespace OHOS
#endif
