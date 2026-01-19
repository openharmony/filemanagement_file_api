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

#include "symlink_core.h"

#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<void> SymlinkCore::DoSymlink(const string &target, const string &srcPath)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> symlinkReq = { new (nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!symlinkReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<void>::Error(ENOMEM);
    }
    int ret = uv_fs_symlink(nullptr, symlinkReq.get(), target.c_str(), srcPath.c_str(), 0, nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a link for old path");
        return FsResult<void>::Error(ret);
    }
    return FsResult<void>::Success();
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS