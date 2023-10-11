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
        HILOGE("Lseek, Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succGetFd, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succGetFd || fd < 0) {
        HILOGE("Lseek, Invalid fd from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    
    auto [succGetOffset, offset] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64();
    if (!succGetOffset) {
        HILOGE("Lseek, Invalid offset from JS second argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int64_t pos = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        pos = ::Lseek(fd, offset, SeekPos::START);
    } else if (funcArg.GetArgc() == NARG_CNT::THREE) {
        auto [succGetWhence, whence] = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(SeekPos::START);
        if (!succGetWhence || whence < SeekPos::START || whence > SeekPos::END) {
            HILOGE("Lseek, Invalid whence from JS third argument");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        pos = ::Lseek(fd, offset, static_cast<SeekPos>(whence));
    }
    
    if (pos < 0) {
        HILOGE("Failed to Lseek");
        NError(errno).ThrowErr(env);
		return nullptr;
    }

    return NVal::CreateInt64(env, pos).val_;
}

} // ModuleFileIO
} // FileManagement
} // OHOS