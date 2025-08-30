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

#include "fs_randomaccessfile.h"

#include <fcntl.h>

#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libfs.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static int DoReadRAF(void* buf, size_t len, int fd, int64_t offset)
{
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> readReq = { new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (readReq == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    int ret = uv_fs_read(nullptr, readReq.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}

static int DoWriteRAF(void* buf, size_t len, int fd, int64_t offset)
{
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> writeReq = { new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (writeReq == nullptr) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    uv_buf_t iov = uv_buf_init(static_cast<char *>(buf), len);
    int ret = uv_fs_write(nullptr, writeReq.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}

FsResult<int32_t> FsRandomAccessFile::GetFD() const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<int32_t>::Error(EIO);
    }
    return FsResult<int32_t>::Success(rafEntity->fd.get()->GetFD());
}

FsResult<int64_t> FsRandomAccessFile::GetFPointer() const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<int64_t>::Error(EIO);
    }
    return FsResult<int64_t>::Success(rafEntity->filePointer);
}

FsResult<void> FsRandomAccessFile::SetFilePointerSync(const int64_t &fp) const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<void>::Error(EIO);
    }
    rafEntity->filePointer = fp;
    return FsResult<void>::Success();
}

static int64_t CalculateOffset(int64_t offset, int64_t fPointer)
{
    if (offset < 0) {
        HILOGD("No specified offset provided");
        offset = fPointer;
    } else {
        offset += fPointer;
    }
    return offset;
}

tuple<bool, void *, size_t, int64_t> ValidReadArg(ArrayBuffer &buffer, const optional<ReadOptions> &options)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;
    void *buf = buffer.buf;
    size_t bufLen = buffer.length;

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

FsResult<int64_t> FsRandomAccessFile::ReadSync(ArrayBuffer &buffer, const optional<ReadOptions> &options) const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<int64_t>::Error(EIO);
    }

    auto [succ, buf, len, offset] = ValidReadArg(buffer, options);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        return FsResult<int64_t>::Error(EINVAL);
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);

    int actLen = DoReadRAF(buf, len, rafEntity->fd.get()->GetFD(), offset);
    if (actLen < 0) {
        HILOGE("Failed to read file for %{public}d", actLen);
        return FsResult<int64_t>::Error(actLen);
    }
    rafEntity->filePointer = offset + actLen;
    return FsResult<int64_t>::Success(static_cast<int64_t>(actLen));
}

tuple<bool, void *, size_t, int64_t> ValidWriteArg(
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
    optional<int64_t> offsetOp = nullopt;
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
            HILOGE("Option.offset shall be positive number");
            return { false, nullptr, 0, offset };
        }
    }
    return { true, buffer, retLen, offset };
}

FsResult<int64_t> FsRandomAccessFile::WriteSync(const string &buffer, const optional<WriteOptions> &options) const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<int64_t>::Error(EIO);
    }

    bool succ = false;
    size_t len = 0;
    int64_t offset = -1;
    void *buf = nullptr;
    size_t bufLen = static_cast<size_t>(buffer.length());
    
    tie(succ, buf, len, offset) = ValidWriteArg(const_cast<void *>(static_cast<const void *>(buffer.c_str())),
        bufLen, options);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        return FsResult<int64_t>::Error(EINVAL);
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    int writeLen = DoWriteRAF(buf, len, rafEntity->fd.get()->GetFD(), offset);
    if (writeLen < 0) {
        return FsResult<int64_t>::Error(writeLen);
    }
    rafEntity->filePointer = offset + writeLen;
    return FsResult<int64_t>::Success(static_cast<int64_t>(writeLen));
}

FsResult<int64_t> FsRandomAccessFile::WriteSync(const ArrayBuffer &buffer, const optional<WriteOptions> &options) const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<int64_t>::Error(EIO);
    }

    bool succ = false;
    size_t len = 0;
    int64_t offset = -1;
    void *buf = nullptr;

    tie(succ, buf, len, offset) = ValidWriteArg(buffer.buf, buffer.length, options);
    if (!succ) {
        HILOGE("Invalid buffer/options");
        return FsResult<int64_t>::Error(EINVAL);
    }
    offset = CalculateOffset(offset, rafEntity->filePointer);
    int writeLen = DoWriteRAF(buf, len, rafEntity->fd.get()->GetFD(), offset);
    if (writeLen < 0) {
        return FsResult<int64_t>::Error(writeLen);
    }
    rafEntity->filePointer = offset + writeLen;
    return FsResult<int64_t>::Success(static_cast<int64_t>(writeLen));
}

static int CloseFd(int fd)
{
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> closeReq = { new (nothrow) uv_fs_t, FsUtils::FsReqCleanup };
    if (!closeReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_close(nullptr, closeReq.get(), fd, nullptr);
    if (ret < 0) {
        HILOGE("Failed to close file with ret: %{private}d", ret);
        return ret;
    }
    return ERRNO_NOERR;
}

FsResult<void> FsRandomAccessFile::CloseSync() const
{
    if (!rafEntity) {
        HILOGE("Failed to get entity of RandomAccessFile");
        return FsResult<void>::Error(EIO);
    }
    auto err = CloseFd(rafEntity->fd.get()->GetFD());
    if (err) {
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}

FsResult<FsRandomAccessFile *> FsRandomAccessFile::Constructor()
{
    auto rafEntity = CreateUniquePtr<RandomAccessFileEntity>();
    if (rafEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsRandomAccessFile *>::Error(ENOMEM);
    }

    FsRandomAccessFile *randomAccessFilePtr = new (nothrow) FsRandomAccessFile(move(rafEntity));
    if (randomAccessFilePtr == nullptr) {
        HILOGE("INNER BUG. Failed to wrap entity for obj RandomAccessFile");
        return FsResult<FsRandomAccessFile *>::Error(EIO);
    }

    return FsResult<FsRandomAccessFile *>::Success(move(randomAccessFilePtr));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS