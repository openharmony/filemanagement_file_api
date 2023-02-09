/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <uv.h>

#include "../../common/napi/uni_header.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
class WatcherHandleDeleter {
public:
    void operator()(uv_fs_event_t *ptr)
    {
        if (ptr == nullptr) {
            return;
        }

        uv_fs_event_stop(ptr);
        uv_handle_t *handle = reinterpret_cast<uv_handle_t *>(ptr);
        uv_close(handle, [](uv_handle_t *handle) { delete handle; });
    }
};

struct WatcherInforArg {
    int events = 0;
    napi_env env = nullptr;
    napi_ref ref = nullptr;
};

struct WatcherEntity {
    std::unique_ptr<WatcherInforArg> data_;
    std::unique_ptr<uv_fs_event_t, WatcherHandleDeleter> fsEventReq_;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif
