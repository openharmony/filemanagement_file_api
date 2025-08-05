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

#include "dup_core.h"

#include <memory>
#include <tuple>
#include <unistd.h>
#include <sys/types.h>

#include "file_entity.h"
#include "file_instantiator.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static bool ValidFd(const int32_t &fd)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return false;
    }
    return true;
}

FsResult<FsFile *> DupCore::DoDup(const int32_t &fd)
{
    if (!ValidFd(fd)) {
        return FsResult<FsFile *>::Error(EINVAL);
    }
    
    int dstFd = dup(fd);
    if (dstFd < 0) {
        HILOGE("Failed to dup fd, errno: %{public}d", errno);
        return FsResult<FsFile *>::Error(errno);
    }
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> readLinkReq = {
        new (std::nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (!readLinkReq) {
        HILOGE("Failed to request heap memory.");
        close(dstFd);
        return FsResult<FsFile *>::Error(ENOMEM);
    }
    string path = "/proc/self/fd/" + to_string(dstFd);
    int ret = uv_fs_readlink(nullptr, readLinkReq.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to readlink fd, ret: %{public}d", ret);
        close(dstFd);
        return FsResult<FsFile *>::Error(ret);
    }
    return FileInstantiator::InstantiateFile(dstFd, string(static_cast<const char *>(readLinkReq->ptr)), false);
}
} // namespace OHOS::FileManagement::ModuleFileIO