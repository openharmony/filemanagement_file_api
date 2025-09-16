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

#include "mkdir_core.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#include "file_fs_trace.h"
#include "filemgmt_libhilog.h"

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "rust_file.h"
#endif

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static int UvAccess(const string &path, int mode)
{
    FileFsTrace traceUvAccess("UvAccess");
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> accessReq = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!accessReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_access(nullptr, accessReq.get(), path.c_str(), mode, nullptr);
    if (FileApiDebug::isLogEnabled) {
        HILOGD("Path is %{public}s", path.c_str());
    }
    return ret;
}

static int MkdirCore(const string &path)
{
    FileFsTrace traceMkdirCore("MkdirCore");
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> mkdirReq = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!mkdirReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_mkdir(nullptr, mkdirReq.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
    if (FileApiDebug::isLogEnabled) {
        HILOGD("Path is %{public}s", path.c_str());
    }
    return ret;
}

static int32_t MkdirExec(const string &path, bool recursion, bool hasOption)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (hasOption) {
        int ret = UvAccess(path, 0);
        if (ret == ERRNO_NOERR) {
            HILOGD("The path already exists");
            return EEXIST;
        }
        if (ret != -ENOENT) {
            HILOGE("Failed to check for illegal path or request for heap memory");
            return ret;
        }
        if (::Mkdirs(path.c_str(), static_cast<MakeDirectionMode>(recursion)) < 0) {
            HILOGD("Failed to create directories, error: %{public}d", errno);
            return errno;
        }
        ret = UvAccess(path, 0);
        if (ret) {
            HILOGE("Failed to verify the result of Mkdirs function");
            return ret;
        }
        return ERRNO_NOERR;
    }
#endif
    int ret = MkdirCore(path);
    if (ret) {
        HILOGD("Failed to create directory");
        return ret;
    }
    return ERRNO_NOERR;
}

FsResult<void> MkdirCore::DoMkdir(const std::string &path, std::optional<bool> recursion)
{
    FileFsTrace traceDoMkdir("DoMkdir");
    bool hasOption = false;
    bool mkdirRecursion = false;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    hasOption = recursion.has_value();
    if (hasOption) {
        mkdirRecursion = recursion.value();
    }
#endif
    auto err = MkdirExec(path, mkdirRecursion, hasOption);
    if (err) {
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS