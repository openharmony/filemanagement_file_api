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

#include "fs_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

tuple<bool, size_t> FsUtils::GetActualLen(size_t bufLen, size_t bufOff, const optional<int64_t> &length)
{
    if (bufLen < bufOff) {
        HILOGE(
            "Illegal parameter value: bufLen (%{public}zu) cannot be less than bufOff (%{public}zu)", bufLen, bufOff);
        return { false, 0 };
    }

    size_t retLen = bufLen - bufOff;

    if (length.has_value()) {
        int64_t opLength = length.value();
        if (opLength < 0 || static_cast<size_t>(opLength) > retLen) {
            HILOGE("Invalid option.length: option.length=%{public}" PRId64 ", retLen=%{public}zu", opLength, retLen);
            return { false, 0 };
        }
        retLen = static_cast<size_t>(opLength);
    }
    return { true, retLen };
}

uint32_t FsUtils::ConvertFlags(const uint32_t &flags)
{
    // default value is usrReadOnly 00
    uint32_t flagsABI = 0;
    flagsABI |= ((flags & USR_WRITE_ONLY) == USR_WRITE_ONLY) ? WRONLY : 0;
    flagsABI |= ((flags & USR_RDWR) == USR_RDWR) ? RDWR : 0;
    flagsABI |= ((flags & USR_CREATE) == USR_CREATE) ? CREATE : 0;
    flagsABI |= ((flags & USR_TRUNC) == USR_TRUNC) ? TRUNC : 0;
    flagsABI |= ((flags & USR_APPEND) == USR_APPEND) ? APPEND : 0;
    flagsABI |= ((flags & USR_NONBLOCK) == USR_NONBLOCK) ? NONBLOCK : 0;
    flagsABI |= ((flags & USR_DIRECTORY) == USR_DIRECTORY) ? DIRECTORY : 0;
    flagsABI |= ((flags & USR_NOFOLLOW) == USR_NOFOLLOW) ? NOFOLLOW : 0;
    flagsABI |= ((flags & USR_SYNC) == USR_SYNC) ? SYNC : 0;
    return flagsABI;
}

void FsUtils::FsReqCleanup(uv_fs_t *req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}

string FsUtils::GetModeFromFlags(const uint32_t &flags)
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

string FsUtils::Decode(const string &uri)
{
    ostringstream outPutStream;
    const int32_t encodeLen = 2;
    size_t index = 0;
    while (index < uri.length()) {
        if (uri[index] == '%') {
            int hex = 0;
            istringstream inputStream(uri.substr(index + 1, encodeLen));
            inputStream >> hex >> hex;
            outPutStream << static_cast<char>(hex);
            index += encodeLen + 1;
        } else {
            outPutStream << uri[index];
            index++;
        }
    }

    return outPutStream.str();
}

} // namespace OHOS::FileManagement::ModuleFileIO
