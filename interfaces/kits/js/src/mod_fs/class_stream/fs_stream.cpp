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

#include "fs_stream.h"

#include <cstring>
#include <tuple>

#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
const string UTF_8 = "utf-8";

std::shared_ptr<FILE> FsStream::GetFilePtr()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!streamEntity) {
        return nullptr;
    }
    return streamEntity->fp;
}

static tuple<bool, size_t, int64_t> ValidWriteArg(const size_t bufLen, const optional<WriteOptions> &options)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;

    if (bufLen > UINT_MAX) {
        HILOGE("The Size of buffer is too large");
        return { false, 0, offset };
    }

    optional<int64_t> lengthOp = nullopt;
    optional<int64_t> offsetOp = nullopt;
    optional<string> encodingOp = nullopt;
    if (options.has_value()) {
        WriteOptions op = options.value();
        lengthOp = op.length;
        offsetOp = op.offset;
        encodingOp = op.encoding;
    }

    tie(succ, retLen) = FsUtils::GetActualLen(bufLen, 0, lengthOp);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, 0, offset };
    }

    if (offsetOp.has_value()) {
        offset = offsetOp.value();
        if (offset < 0) {
            HILOGE("option.offset shall be positive number");
            return { false, 0, offset };
        }
    }

    if (encodingOp.has_value()) {
        if (encodingOp.value() != UTF_8) {
            HILOGE("option.encoding shall be utf-8");
            return { false, 0, offset };
        }
    }
    return { true, retLen, offset };
}

static tuple<bool, size_t, int64_t> ValidReadArg(const size_t bufLen, const optional<ReadOptions> &options)
{
    size_t retLen = 0;
    int64_t offset = -1;
    bool succ = false;

    if (bufLen > UINT_MAX) {
        HILOGE("The Size of buffer is too large");
        return { false, 0, offset };
    }

    optional<int64_t> lengthOp = nullopt;
    optional<int64_t> offsetOp = nullopt;
    if (options.has_value()) {
        ReadOptions op = options.value();
        lengthOp = op.length;
        offsetOp = op.offset;
    }

    tie(succ, retLen) = FsUtils::GetActualLen(bufLen, 0, lengthOp);
    if (!succ) {
        HILOGE("Failed to get actual length");
        return { false, 0, offset };
    }

    if (offsetOp.has_value()) {
        offset = offsetOp.value();
        if (offset < 0) {
            HILOGE("option.offset shall be positive number");
            return { false, 0, offset };
        }
    }
    return { true, retLen, offset };
}

FsResult<size_t> FsStream::Write(const ArrayBuffer &buf, const optional<WriteOptions> &options)
{
    FileFsTrace traceFsStreamWrite("FsStreamWrite");
    auto fp = GetFilePtr();
    if (!fp) {
        HILOGE("Failed to get file ptr");
        return FsResult<size_t>::Error(EIO);
    }

    auto [succ, retLen, offset] = ValidWriteArg(buf.length, options);
    if (!succ) {
        HILOGE("Invalid options");
        return FsResult<size_t>::Error(EINVAL);
    }

    if (offset >= 0) {
        FileFsTrace traceFseek("fseek");
        int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
        traceFseek.End();
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return FsResult<size_t>::Error(errno);
        }
    }

    FileFsTrace traceFwrite("fwrite");
    size_t writeLen = fwrite(buf.buf, 1, retLen, fp.get());
    traceFwrite.End();
    if ((writeLen == 0) && (writeLen != retLen)) {
        HILOGE("Failed to fwrite stream");
        return FsResult<size_t>::Error(EIO);
    }
    return FsResult<size_t>::Success(writeLen);
}

FsResult<size_t> FsStream::Write(const string &buf, const optional<WriteOptions> &options)
{
    FileFsTrace traceFsStreamWrite("FsStreamWrite");
    auto fp = GetFilePtr();
    if (!fp) {
        HILOGE("Failed to get file ptr");
        return FsResult<size_t>::Error(EIO);
    }

    size_t bufLen = static_cast<size_t>(buf.length());

    auto [succ, retLen, offset] = ValidWriteArg(bufLen, options);
    if (!succ) {
        HILOGE("Invalid options");
        return FsResult<size_t>::Error(EINVAL);
    }

    if (offset >= 0) {
        FileFsTrace traceFseek("fseek");
        int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
        traceFseek.End();
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return FsResult<size_t>::Error(errno);
        }
    }

    FileFsTrace traceFwrite("fwrite");
    size_t writeLen = fwrite(buf.c_str(), 1, retLen, fp.get());
    traceFwrite.End();
    if ((writeLen == 0) && (writeLen != retLen)) {
        HILOGE("Failed to fwrite stream");
        return FsResult<size_t>::Error(EIO);
    }
    return FsResult<size_t>::Success(writeLen);
}

FsResult<size_t> FsStream::Read(ArrayBuffer &buf, const optional<ReadOptions> &options)
{
    auto fp = GetFilePtr();
    if (!fp) {
        HILOGE("Failed to get file ptr");
        return FsResult<size_t>::Error(EIO);
    }

    auto [succ, retLen, offset] = ValidReadArg(buf.length, options);
    if (!succ) {
        HILOGE("Invalid options");
        return FsResult<size_t>::Error(EINVAL);
    }

    if (offset >= 0) {
        int ret = fseek(fp.get(), static_cast<long>(offset), SEEK_SET);
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return FsResult<size_t>::Error(errno);
        }
    }

    size_t actLen = fread(buf.buf, 1, retLen, fp.get());
    if ((actLen != static_cast<size_t>(retLen) && !feof(fp.get())) || ferror(fp.get())) {
        HILOGE("Invalid buffer size or pointer, actlen: %{public}zu", actLen);
        return FsResult<size_t>::Error(EIO);
    }

    return FsResult<size_t>::Success(actLen);
}

FsResult<void> FsStream::Flush()
{
    FileFsTrace traceFsStreamFlush("FsStreamFlush");
    auto fp = GetFilePtr();
    if (fp == nullptr) {
        HILOGE("Failed to get entity of Stream");
        return FsResult<void>::Error(EIO);
    }

    FileFsTrace traceFflush("fflush");
    int ret = fflush(fp.get());
    traceFflush.End();
    if (ret < 0) {
        HILOGE("Failed to fflush file in the stream, ret: %{public}d", ret);
        return FsResult<void>::Error(errno);
    }

    return FsResult<void>::Success();
}

FsResult<void> FsStream::Close()
{
    FileFsTrace traceFsStreamClose("FsStreamClose");
    if (!streamEntity) {
        HILOGE("Failed to get entity of Stream, may closed twice");
        return FsResult<void>::Error(EIO);
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        streamEntity = nullptr;
    }
    return FsResult<void>::Success();
}

FsResult<int64_t> FsStream::Seek(const int64_t &offset, const optional<int32_t> &typeOpt)
{
    int whence = SEEK_SET;

    auto fp = GetFilePtr();
    if (fp == nullptr) {
        HILOGE("Failed to get file ptr");
        return FsResult<int64_t>::Error(ENOENT);
    }

    if (typeOpt.has_value()) {
        int pos = typeOpt.value();
        if (pos < SEEK_SET || pos > SEEK_END) {
            HILOGE("Invalid whence");
            return FsResult<int64_t>::Error(EINVAL);
        }
        whence = pos;
    }

    if (offset >= 0) {
        int ret = fseek(fp.get(), static_cast<long>(offset), whence);
        if (ret < 0) {
            HILOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return FsResult<int64_t>::Error(errno);
        }
    }

    int64_t res = ftell(fp.get());
    if (res < 0) {
        HILOGE("Failed to tell, error:%{public}d", errno);
        return FsResult<int64_t>::Error(errno);
    }

    return FsResult<int64_t>::Success(res);
}

FsResult<FsStream *> FsStream::Constructor()
{
    auto rafEntity = CreateUniquePtr<StreamEntity>();
    if (rafEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsStream *>::Error(ENOMEM);
    }
    FsStream *fsStreamPtr = new FsStream(move(rafEntity));

    if (fsStreamPtr == nullptr) {
        HILOGE("Failed to create FsStream object on heap.");
        return FsResult<FsStream *>::Error(ENOMEM);
    }

    return FsResult<FsStream *>::Success(move(fsStreamPtr));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
