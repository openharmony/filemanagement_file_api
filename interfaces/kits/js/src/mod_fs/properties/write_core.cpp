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

#include "write_core.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, void *, size_t, int64_t> ValidWriteArg(
    void *buffer, const size_t bufLen, const optional<WriteOptions> &options)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;

    if (bufLen > UINT_MAX) {
        HILOGE("The Size of buffer is too large");
        return { false, nullptr, 0, offset };
    }

    optional<size_t> lengthOp = nullopt;
    optional<size_t> offsetOp = nullopt;
    if (options.has_value()) {
        WriteOptions op = options.value();
        lengthOp = op.length;
        offsetOp = op.offset;
    }

    tie(succ, retLen) = FsUtils::GetActualLen(bufLen, 0, lengthOp);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, 0, offset };
    }

    if (offsetOp.has_value()) {
        offset = offsetOp.value();
        if (offset < 0) {
            HILOGE("option.offset shall be positive number");
            return { false, nullptr, 0, offset };
        }
    }
    return { true, buffer, retLen, offset };
}

FsResult<int64_t> WriteCore::DoWrite(const int32_t fd, const string &buffer, const optional<WriteOptions> &options)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return FsResult<int64_t>::Error(EINVAL);
    }

    bool succ = false;
    size_t len = 0;
    int64_t offset = -1;
    void *buf = const_cast<void *>(static_cast<const void *>(buffer.c_str()));
    size_t bufLen = static_cast<size_t>(buffer.length());

    tie(succ, buf, len, offset) = ValidWriteArg(buf, bufLen, options);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return FsResult<int64_t>::Error(EINVAL);
    }
    return DoWrite(fd, buf, len, offset);
}

FsResult<int64_t> WriteCore::DoWrite(const int32_t fd, const ArrayBuffer &buffer, const optional<WriteOptions> &options)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return FsResult<int64_t>::Error(EINVAL);
    }

    bool succ = false;
    size_t len = 0;
    int64_t offset = -1;
    void *buf = nullptr;

    tie(succ, buf, len, offset) = ValidWriteArg(buffer.buf, buffer.length, options);
    if (!succ) {
        HILOGE("Failed to resolve buf and options");
        return FsResult<int64_t>::Error(EINVAL);
    }
    return DoWrite(fd, buf, len, offset);
}

FsResult<int64_t> WriteCore::DoWrite(const int32_t fd, void *buf, const size_t len, const int64_t offset)
{
    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<unsigned int>(len));
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> write_req = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!write_req) {
        return FsResult<int64_t>::Error(ENOMEM);
    }
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to write file for %{public}d", ret);
        return FsResult<int64_t>::Error(ret);
    }
    return FsResult<int64_t>::Success(static_cast<int64_t>(ret));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS