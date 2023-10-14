/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "rust_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value Lseek::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succGetFd, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succGetFd || fd < 0) {
        HILOGE("Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    
    auto [succGetOffset, offset] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64();
    if (!succGetOffset) {
        HILOGE("Invalid offset from JS second argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    SeekPos whence = SeekPos::START;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        auto [succGetWhence, pos] = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(SeekPos::START);
        if (!succGetWhence || pos < SeekPos::START || pos > SeekPos::END) {
            HILOGE("Invalid whence from JS third argument");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        whence = static_cast<SeekPos>(pos);
    }

    int64_t ret = ::Lseek(fd, offset, whence);
    if (ret < 0) {
        HILOGE("Failed to lseek, error:%{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, ret).val_;
}

} // ModuleFileIO
} // FileManagement
} // OHOS