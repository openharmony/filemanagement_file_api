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

#ifndef OHOS_FILE_FS_WATCHER_IMPL_H
#define OHOS_FILE_FS_WATCHER_IMPL_H

#include <mutex>
#include <sys/inotify.h>
#include <unordered_map>
#include <unordered_set>

#include "uni_error.h"
#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "cj_lambda.h"
#include "singleton.h"

namespace OHOS {
namespace CJSystemapi {
constexpr int BUF_SIZE = 1024;

struct CWatchEvent {
    const char* fileName;
    uint32_t event;
    uint32_t cookie;
};
struct WatcherInfoArg {
    std::string fileName = "";
    uint32_t events = 0;
    int wd = -1;
    std::function<void(CWatchEvent)> watchCallback_;
    explicit WatcherInfoArg(void (*callback)(CWatchEvent))
    {
        watchCallback_ = CJLambda::Create(callback);
    }
    ~WatcherInfoArg() = default;
};

class WatcherImpl : public OHOS::FFI::FFIData, public Singleton<WatcherImpl> {
public:
    std::shared_ptr<WatcherInfoArg> data_;
    WatcherImpl();
    int32_t GetNotifyId();
    bool InitNotify();
    bool AddWatcherInfo(const std::string &fileName, std::shared_ptr<WatcherInfoArg> arg);
    bool CheckEventValid(const uint32_t &event);
    int32_t StartNotify();
    void GetNotifyEvent();
    int32_t StopNotify();

    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }
private:
    uint32_t RemoveWatcherInfo(std::shared_ptr<WatcherInfoArg> arg);
    std::tuple<bool, int> CheckEventWatched(const std::string &fileName, const uint32_t &event);
    void NotifyEvent(const struct inotify_event *event);
    int CloseNotifyFd();
    int NotifyToWatchNewEvents(const std::string &fileName, const int &wd, const uint32_t &watchEvents);

private:
    static std::mutex watchMutex_;
    bool run_ = false;
    int32_t notifyFd_ = -1;
    std::unordered_set<std::shared_ptr<WatcherInfoArg>> watcherInfoSet_;
    std::unordered_map<std::string, std::pair<int, uint32_t>> wdFileNameMap_;

    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType = OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("WatcherImpl");
        return &runtimeType;
    }
};
} // OHOS::FileManagement::ModuleFileIO
}

#endif // OHOS_FILE_FS_IMPL_H