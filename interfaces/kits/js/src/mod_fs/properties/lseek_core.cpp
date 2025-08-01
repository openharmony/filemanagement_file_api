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

#include "lseek_core.h"

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<int64_t> LseekCore::DoLseek(const int32_t &fd, const int64_t &offset,
                                     const optional<SeekPos> &pos)
{
    if (fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        return FsResult<int64_t>::Error(EINVAL);
    }
    
    SeekPos whence = SeekPos::START;
    if (pos.has_value()) {
        if (pos.value() < SeekPos::START || pos.value() > SeekPos::END) {
            HILOGE("Invalid whence from JS third argument");
            return FsResult<int64_t>::Error(EINVAL);
        }
        whence = pos.value();
    }

    int64_t ret = ::Lseek(fd, offset, whence);
    if (ret < 0) {
        HILOGE("Failed to lseek, error:%{public}d", errno);
        return FsResult<int64_t>::Error(errno);
    }
    return FsResult<int64_t>::Success(ret);
}

} // ModuleFileIO
} // FileManagement
} // OHOS