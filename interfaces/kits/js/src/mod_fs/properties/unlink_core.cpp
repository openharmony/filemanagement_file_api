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

#include "unlink_core.h"

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<void> UnlinkCore::DoUnlink(const std::string &src)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> unlink_req = {
        new uv_fs_t, FsUtils::FsReqCleanup };
    if (!unlink_req) {
        HILOGE("Failed to request heap memory.");
        return FsResult<void>::Error(ENOMEM);
    }
    int ret = uv_fs_unlink(nullptr, unlink_req.get(), src.c_str(), nullptr);
    if (ret < 0) {
        HILOGD("Failed to unlink with path");
        return FsResult<void>::Error(ret);
    }

    return FsResult<void>::Success();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS