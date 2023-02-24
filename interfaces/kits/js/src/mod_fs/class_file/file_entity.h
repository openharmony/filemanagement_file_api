/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FILE_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FILE_ENTITY_H

#include <cinttypes>
#include <iostream>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fd_guard.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
struct FileEntity {
    std::unique_ptr<DistributedFS::FDGuard> fd_ = { nullptr };
    std::string path_;
    std::string uri_;

    virtual ~FileEntity()
    {
        if (!fd_.get()) {
            return;
        }
        int32_t fd = fd_.get()->GetFD();
        int ret = flock(fd, LOCK_UN);
        if (ret == 0) {
            struct stat buf;
            if (fstat(fd, &buf) == 0) {
                HILOGI("Unlock succeeded inode = %{public}" PRIu64, buf.st_ino);
            } else {
                HILOGI("Failed to get inode number");
            }
        }
    }
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILE_FILE_ENTITY_H