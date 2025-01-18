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

#include "statvfs_impl.h"
#include <cinttypes>
#include <sys/statvfs.h>
#include "uni_error.h"

namespace OHOS {
namespace CJSystemapi {
namespace Statvfs {
    std::tuple<int32_t, int64_t> StatvfsImpl::GetFreeSize(const char* path)
    {
        struct statvfs diskInfo;
        int ret = statvfs(path, &diskInfo);
        if (ret != 0) {
            return {FileFs::GetErrorCode(errno), 0};
        }
        unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                      static_cast<unsigned long long>(diskInfo.f_bfree);
        return {SUCCESS_CODE, static_cast<int64_t>(freeSize)};
    }

    std::tuple<int32_t, int64_t> StatvfsImpl::GetTotalSize(const char* path)
    {
        struct statvfs diskInfo;
        int ret = statvfs(path, &diskInfo);
        if (ret != 0) {
            return {FileFs::GetErrorCode(errno), 0};
        }
        unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                      static_cast<unsigned long long>(diskInfo.f_blocks);
        return {SUCCESS_CODE, static_cast<int64_t>(totalSize)};
    }
} // Statvfs
} // CJSystemapi
} // namespace OHOS