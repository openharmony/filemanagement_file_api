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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCHER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCHER_H

#include "filemgmt_libfs.h"
#include "fs_watch_entity.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {

class FsWatcher {
public:
    static FsResult<FsWatcher *> Constructor();
    FsResult<void> Start();
    FsResult<void> Stop();

    FsWatchEntity *GetWatchEntity() const
    {
        return watchEntity.get();
    }

    FsWatcher(const FsWatcher &) = delete;
    FsWatcher &operator=(const FsWatcher &) = delete;

    FsWatcher(FsWatcher &&) noexcept = default;
    FsWatcher &operator=(FsWatcher &&) noexcept = default;

    ~FsWatcher() {
        HILOGE("~FsWatcher success.");
    }

private:
    unique_ptr<FsWatchEntity> watchEntity;
    explicit FsWatcher(unique_ptr<FsWatchEntity> entity) : watchEntity(move(entity)) {}
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_WATCHER_FS_WATCHER_H
