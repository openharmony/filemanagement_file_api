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

#include "fsync_core.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

FsResult<void> FsyncCore::DoFsync(const int32_t &fd)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> fsyncReq = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!fsyncReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<void>::Error(ENOMEM);
    }
    int ret = uv_fs_fsync(nullptr, fsyncReq.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to transfer data associated with file descriptor: %{public}d", fd);
        return FsResult<void>::Error(ret);
    }
    
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::ModuleFileIO