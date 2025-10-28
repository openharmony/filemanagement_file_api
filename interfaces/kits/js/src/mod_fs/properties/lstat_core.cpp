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

#include "lstat_core.h"

#include <memory>
#include <tuple>

#include <securec.h>

#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "stat_instantiator.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
const std::string SCHEME_FILE = "file";

static string ParsePath(const string &pathStr)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    if (pathStr.find("://") != string::npos) {
        Uri uri(pathStr);
        string uriType = uri.GetScheme();
        if (uriType == SCHEME_FILE) {
            AppFileService::ModuleFileUri::FileUri fileUri(pathStr);
            string realPath = fileUri.GetRealPath();
            return realPath;
        }
    }
#endif
    return pathStr;
}

FsResult<FsStat *> LstatCore::DoLstat(const string &path)
{
    string pathStr = ParsePath(path);
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> lstat_req = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!lstat_req) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    int ret = uv_fs_lstat(nullptr, lstat_req.get(), pathStr.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get stat of file, ret: %{public}d", ret);
        return FsResult<FsStat *>::Error(ret);
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    size_t length = pathStr.length() + 1;
    auto chars = std::make_unique<char[]>(length);
    ret = strncpy_s(chars.get(), length, pathStr.c_str(), length - 1);
    if (ret != EOK) {
        HILOGE("Copy file path failed!");
        return FsResult<FsStat *>::Error(ret);
    }
    struct FileInfo info = { true, move(chars), {} };
    auto arg = CreateSharedPtr<FileInfo>(move(info));
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    auto stat = StatInstantiator::InstantiateStat(lstat_req->statbuf, arg);
#else
    auto stat = StatInstantiator::InstantiateStat(lstat_req->statbuf);
#endif
    if (stat == nullptr) {
        HILOGE("Failed to InstantiateStat.");
        return FsResult<FsStat *>::Error(ENOMEM);
    }
    return FsResult<FsStat *>::Success(stat);
}

} // namespace OHOS::FileManagement::ModuleFileIO