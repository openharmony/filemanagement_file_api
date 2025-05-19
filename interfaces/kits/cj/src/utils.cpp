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

#include "utils.h"
#include "cj_common_ffi.h"
#include "macro.h"
#include "uni_error.h"

namespace OHOS {
namespace CJSystemapi {

unsigned int CommonFunc::ConvertCjFlags(unsigned int &flags)
{
    // default value is usrReadOnly 00
    unsigned int flagsABI = 0;
    flagsABI |= ((flags & USR_WRITE_ONLY) == USR_WRITE_ONLY) ? WRONLY : 0;
    flagsABI |= ((flags & USR_RDWR) == USR_RDWR) ? RDWR : 0;
    flagsABI |= ((flags & USR_CREATE) == USR_CREATE) ? CREATE : 0;
    flagsABI |= ((flags & USR_TRUNC) == USR_TRUNC) ? TRUNC : 0;
    flagsABI |= ((flags & USR_APPEND) == USR_APPEND) ? APPEND : 0;
    flagsABI |= ((flags & USR_NONBLOCK) == USR_NONBLOCK) ? NONBLOCK : 0;
    flagsABI |= ((flags & USR_DIRECTORY) == USR_DIRECTORY) ? DIRECTORY : 0;
    flagsABI |= ((flags & USR_NOFOLLOW) == USR_NOFOLLOW) ? NOFOLLOW : 0;
    flagsABI |= ((flags & USR_SYNC) == USR_SYNC) ? SYNC : 0;
    flags = flagsABI;
    return flagsABI;
}
using namespace std;

void CommonFunc::FsReqCleanup(uv_fs_t* req)
{
    if (req) {
        uv_fs_req_cleanup(req);
        delete req;
        req = nullptr;
    }
}

string CommonFunc::GetModeFromFlags(unsigned int flags)
{
    const string readMode = "r";
    const string writeMode = "w";
    const string appendMode = "a";
    const string truncMode = "t";
    string mode = readMode;
    mode += (((flags & O_RDWR) == O_RDWR) ? writeMode : "");
    mode = (((flags & O_WRONLY) == O_WRONLY) ? writeMode : mode);
    if (mode != readMode) {
        mode += ((flags & O_TRUNC) ? truncMode : "");
        mode += ((flags & O_APPEND) ? appendMode : "");
    }
    return mode;
}

std::tuple<int, void *, size_t, int64_t> CommonFunc::GetWriteArg(
    void *buffer, int64_t length, int64_t offset, const std::string& encode)
{
    if (buffer == nullptr) {
        return { EINVAL, nullptr, 0, offset };
    }
    if (length > UINT_MAX) {
        LOGE("The Size of buffer is too large");
        return { EINVAL, nullptr, 0, offset };
    }
    if (offset < 0) {
        LOGE("option.offset shall be positive number");
        return { EINVAL, nullptr, 0, offset };
    }
    return { SUCCESS_CODE, buffer, length, offset };
}

}
}