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

namespace {
const vector<string> PUBLIC_DIR_PATHS = { "/Documents" };
}

tuple<bool, void *, int64_t> FsUtils::GetActualBuf(void *rawBuf, size_t bufLen, const optional<int64_t> &offset)
{
    void *realBuf = nullptr;
    int64_t opOffset = 0;
    if (offset.has_value()) {
        opOffset = offset.value();
        if (opOffset < 0) {
            HILOGE("Invalid option.offset, positive integer is desired");
            return { false, nullptr, opOffset };
        } else if (opOffset > static_cast<int64_t>(bufLen)) {
            HILOGE("Invalid option.offset, buffer limit exceeded");
            return { false, nullptr, opOffset };
        } else {
            realBuf = static_cast<uint8_t *>(rawBuf) + opOffset;
        }
    } else {
        realBuf = rawBuf;
    }

    return { true, realBuf, opOffset };
}

tuple<bool, size_t> FsUtils::GetActualLen(size_t bufLen, size_t bufOff, const optional<int64_t> &length)
{
    size_t retLen = bufLen - bufOff;

    if (length.has_value()) {
        int64_t opLength = length.value();
        if (opLength < 0 || static_cast<size_t>(opLength) > retLen) {
            HILOGE("Invalid option.length");
            return { false, 0 };
        }
        retLen = static_cast<size_t>(opLength);
    }
    return { true, retLen };
}

uint32_t FsUtils::ConvertFlags(const uint32_t &flags)
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

bool FsUtils::CheckPublicDirPath(const string &sandboxPath)
{
    for (const string &path : PUBLIC_DIR_PATHS) {
        if (sandboxPath.find(path) == 0) {
            return true;
        }
    }
    return false;
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
