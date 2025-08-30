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

#include "utimes_core.h"

#include <unistd.h>
 
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
 
FsResult<void> UtimesCore::DoUtimes(const string &path, const double mtime)
{
    if (mtime < 0) {
        HILOGE("Invalid mtime");
        return FsResult<void>::Error(EINVAL);
    }
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> statReq = { new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (!statReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<void>::Error(ENOMEM);
    }
    int ret = uv_fs_stat(nullptr, statReq.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of the file by path");
        return FsResult<void>::Error(ret);
    }
 
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> utimesReq = {
        new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (!utimesReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<void>::Error(ENOMEM);
    }
 
    double atime = static_cast<double>(statReq->statbuf.st_atim.tv_sec) +
        static_cast<double>(statReq->statbuf.st_atim.tv_nsec) / NS;
    ret = uv_fs_utime(nullptr, utimesReq.get(), path.c_str(), atime, mtime / MS, nullptr);
    if (ret < 0) {
        HILOGE("Failed to chang mtime of the file for %{public}d", ret);
        return FsResult<void>::Error(ret);
    }
    return FsResult<void>::Success();
}
} // ModuleFileIO
} // FileManagement
} // OHOS