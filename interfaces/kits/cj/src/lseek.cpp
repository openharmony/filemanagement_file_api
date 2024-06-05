/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "lseek.h"

#include "stat_impl.h"
#include "macro.h"
#include "n_error.h"
#include "file_utils.h"
#include "rust_file.h"

namespace OHOS {
namespace CJSystemapi {
using namespace std;
using namespace OHOS::FileManagement::LibN;

RetDataI64 LseekImpl::Lseek(int32_t fd, int64_t offset, int pos)
{
    LOGI("FS_TEST:: LseekImpl::Lseek start");
    RetDataI64 ret = { .code = EINVAL, .data = 0 };
    if (fd < 0) {
        LOGE("Invalid fd");
        return ret;
    }

    SeekPos whence = static_cast<SeekPos>(pos);

    int64_t seekRet = ::Lseek(fd, offset, whence);
    if (seekRet < 0) {
        LOGE("Failed to lseek, error:%{public}d", errno);
        ret.code = errno;
        return ret;
    }
    ret.code = SUCCESS_CODE;
    ret.data = seekRet;
    return ret;
}

} // CJSystemapi
} // OHOS
