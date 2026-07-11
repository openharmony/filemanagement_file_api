/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swapfs_err_mapper.h"

namespace OHOS::FileManagement::Swapfs {
int MapErrno(int err, SwapfsErrContext context)
{
    switch (err) {
        case EINVAL:
            return SWAPFS_E_INVAL;
        case EACCES:
        case EPERM:
            return SWAPFS_E_ACCES;
        case ENOSPC:
            return SWAPFS_E_NOSPC;
        case EDQUOT:
            return SWAPFS_E_QUOTA_EXCEEDED;
        case EBUSY:
            return SWAPFS_E_BUSY;
        case EIO:
            return SWAPFS_E_IO_ERROR;
        case ENOENT:
        case ENOTDIR:
            if (context == SwapfsErrContext::PATH_OPERATION) {
                return SWAPFS_E_PATH_UNAVAILABLE;
            }
            if (context == SwapfsErrContext::IO_OPERATION) {
                return SWAPFS_E_IO_ERROR;
            }
            return SWAPFS_E_KEY_NOT_FOUND;
        case ENOMEM:
            return SWAPFS_E_NOMEM;
        default:
            return SWAPFS_E_IO_ERROR;
    }
}
}
