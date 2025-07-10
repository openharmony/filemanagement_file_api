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

#include "read_core.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libfs.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static tuple<bool, void *, size_t, int64_t> ValidReadArg(ArrayBuffer &arrayBuffer, const optional<ReadOptions> &options)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;
    void *buf = arrayBuffer.buf;
    size_t bufLen = arrayBuffer.length;

    if (bufLen > UINT_MAX) {
        HILOGE("Invalid arraybuffer");
        return { false, nullptr, retLen, offset };
    }
    optional<size_t> lengthOp = nullopt;
    optional<int64_t> offsetOp = nullopt;
    if (options.has_value()) {
        ReadOptions op = options.value();
        lengthOp = op.length;
        offsetOp = op.offset;
    }
    tie(succ, retLen) = FsUtils::GetActualLen(bufLen, 0, lengthOp);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, nullptr, retLen, offset };
    }
    if (offsetOp.has_value()) {
        offset = offsetOp.value();
        if (offset < 0) {
            HILOGE("option.offset shall be positive number");
            return { false, nullptr, retLen, offset };
        }
    }
    return { true, buf, retLen, offset };
}

FsResult<int64_t> ReadCore::DoRead(const int32_t &fd, ArrayBuffer &arrayBuffer, const optional<ReadOptions> &options)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return FsResult<int64_t>::Error(EINVAL);
    }

    auto [succ, buf, len, offset] = ValidReadArg(arrayBuffer, options);
    if (!succ) {
        return FsResult<int64_t>::Error(EINVAL);
    }

    uv_buf_t buffer = uv_buf_init(static_cast<char *>(buf), static_cast<uint32_t>(len));
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> readReq = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!readReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<int64_t>::Error(ENOMEM);
    }
    int ret = uv_fs_read(nullptr, readReq.get(), fd, &buffer, 1, offset, nullptr);
    if (ret < 0) {
        HILOGE("Failed to read file for %{public}d", ret);
        return FsResult<int64_t>::Error(ret);
    }

    return FsResult<int64_t>::Success(static_cast<int64_t>(ret));
}

} // namespace OHOS::FileManagement::ModuleFileIO