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

#include "statvfs_core.h"

#include <sys/statvfs.h>
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStatvfs {
using namespace std;

FsResult<int64_t> StatvfsCore::DoGetFreeSize(const string &path)
{
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != 0) {
        return FsResult<int64_t>::Error(errno);
    }
    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bfree);
    return FsResult<int64_t>::Success(static_cast<int64_t>(freeSize));
}

FsResult<int64_t> StatvfsCore::DoGetTotalSize(const string &path)
{
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != 0) {
        return FsResult<int64_t>::Error(errno);
    }
    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                   static_cast<unsigned long long>(diskInfo.f_blocks);
    return FsResult<int64_t>::Success(static_cast<int64_t>(totalSize));
}

} // namespace ModuleStatfs
} // namespace FileManagement
} // namespace OHOS