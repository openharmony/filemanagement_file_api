/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fdatasync.h"
#include "file_fs_impl.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace OHOS {
namespace CJSystemapi {

using namespace std;

int FdatasyncImpl::Fdatasync(int32_t fd)
{
    LOGI("FS_TEST:: FdatasyncImpl::Fdatasync start");
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> fdatasync_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!fdatasync_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_fdatasync(nullptr, fdatasync_req.get(), fd, nullptr);
    if (ret < 0) {
        LOGE("Failed to transfer data associated with file descriptor: %{public}d, ret:%{public}d", fd, ret);
        return ret;
    }
    LOGI("FS_TEST:: FdatasyncImpl::Fdatasync success");
    return ret;
}

}
}