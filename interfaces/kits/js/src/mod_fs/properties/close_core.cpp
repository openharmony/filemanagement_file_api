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

#include <cstdio>
#include <cstring>
#include <tuple>
#include <unistd.h>

#include "fdtag_func.h"
#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static int32_t CloseFd(int fd)
{
    FileFsTrace traceCloseFd("CloseFd");
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> closeReq = { new (nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!closeReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_close(nullptr, closeReq.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to close file with ret: %{public}d", ret);
        return ret;
    }
    return ERRNO_NOERR;
}

static int32_t CloseFdWithFdsan(const int fd, const bool isFd, const uint64_t fileTag)
{
    FileFsTrace traceCloseFd("CloseFd");
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    if (fd >= OVERFLOW_END) {
        return CloseFd(fd);
    }
 
    if (isFd) {
        FdTagFunc::SetFdTag(fd, 0);
        return CloseFd(fd);
    } else {
        auto tag = FdTagFunc::GetFdTag(fd);
        if (tag <= 0 || tag != fileTag) {
            tag = fileTag|PREFIX_ADDR;
        } else {
            tag = 0;
        }
        FdTagFunc::SetFdTag(fd, 0);
        int ret = fdsan_close_with_tag(fd, tag);
        if (ret < 0) {
            HILOGE("Failed to close file with errno: %{public}d", ret);
            return ret;
        }
    }
    return ERRNO_NOERR;
#else
    return CloseFd(fd);
#endif
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
    FileFsTrace traceDoClose("DoClose");
    if (!ValidFd(fd)) {
        return FsResult<void>::Error(EINVAL);
    }
    auto err = CloseFdWithFdsan(fd, true, 0);
    if (err) {
        HILOGE("Failed to close fd");
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}

FsResult<void> CloseCore::DoClose(FsFile *file)
{
    FileFsTrace traceDoClose("DoClose");
    auto ret = file->GetFD();
    if (!ret.IsSuccess()) {
        HILOGE("Failed to get fd");
        return FsResult<void>::Error(EINVAL);
    }
    uint64_t fileTag = reinterpret_cast<std::uintptr_t>(file->GetFileEntity());
    auto err = CloseFdWithFdsan(ret.GetData().value(), false, fileTag);
    if (err) {
        HILOGE("Failed to close file");
        return FsResult<void>::Error(err);
    }

    file->RemoveEntity();
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::ModuleFileIO
