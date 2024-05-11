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

#include "fsync.h"
#include "file_fs_impl.h"

#include <cstring>
#include <unistd.h>


namespace OHOS {
namespace CJSystemapi {

using namespace std;

int FsyncImpl::Fsync(int32_t fd)
{
    LOGI("FS_TEST::FsyncImpl::Fsync start");
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> fsync_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!fsync_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_fsync(nullptr, fsync_req.get(), fd, nullptr);
    if (ret < 0) {
        LOGE("Failed to transfer data associated with file descriptor: %{public}d", fd);
        return ret;
    }
    LOGI("FS_TEST::FsyncImpl::Fsync success");
    return ret;
}

} // namespace CJSystemapi
} // namespace OHOS
