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

#include "close_core.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static int32_t CloseFd(int fd)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> close_req = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!close_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_close(nullptr, close_req.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to close file with ret: %{public}d", ret);
        return ret;
    }
    return ERRNO_NOERR;
}

static bool ValidFd(const int32_t &fd)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return false;
    }
    return true;
}

FsResult<void> CloseCore::DoClose(const int32_t &fd)
{
    if (!ValidFd(fd)) {
        return FsResult<void>::Error(EINVAL);
    }
    auto err = CloseFd(fd);
    if (err) {
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::ModuleFileIO
