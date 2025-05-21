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

#include "stream_impl.h"
#include <memory>
#include <tuple>
#include "securec.h"
#include "utils.h"

using namespace std;

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

std::tuple<int, size_t> GetActualLen(size_t bufLen, size_t bufOff, int64_t offset, int64_t length)
{
    size_t retLen = bufLen - bufOff;

    if (length == 0) {
        return { SUCCESS_CODE, retLen };
    }

    if (length < 0 || static_cast<size_t>(length) > retLen) {
        LOGE("Invalid option length, length: %{public}" PRId64 ", retLen: %{public}zu", length, retLen);
        return { EINVAL, 0 };
    }
    retLen = static_cast<size_t>(length);
    return { SUCCESS_CODE, retLen };
}

tuple<int, std::unique_ptr<char[]>, size_t, int64_t> GetReadArg(size_t bufLen, int64_t length, int64_t offset)
{
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(bufLen);

    auto [state, retLen] = GetActualLen(bufLen, 0, offset, length);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to get actual length");
        return { EINVAL, nullptr, 0, 0 };
    }

    if (offset < 0) {
        LOGE("option.offset shall be positive number");
        return { EINVAL, nullptr, 0, 0 };
    }

    return { SUCCESS_CODE, move(buf), retLen, offset };
}

int StreamImpl::Close()
{
    if (!fp_) {
        LOGE("close false, fp is null");
        return GetErrorCode(EIO);
    }
    fp_.reset();
    return SUCCESS_CODE;
}

int StreamImpl::Flush()
{
    if (!fp_) {
        LOGE("flush false, fp is null");
        return GetErrorCode(EIO);
    }
    int ret = fflush(fp_.get());
    if (ret < 0) {
        LOGE("Failed to fflush file in the stream, ret: %{public}d", ret);
        return GetErrorCode(errno);
    }
    return SUCCESS_CODE;
}

tuple<int, int64_t> ReadImpl(std::unique_ptr<char[]> &buf, size_t len, FILE* filp, uint8_t* buffer)
{
    size_t actLen = fread(buf.get(), sizeof(char), len, filp);
    if ((actLen != static_cast<size_t>(len) && !feof(filp)) || ferror(filp)) {
        LOGE("Invalid buffer size and pointer, actlen: %{public}zu", actLen);
        return {GetErrorCode(EIO), 0};
    }
    if (actLen != 0) {
        int ret = memcpy_s(buffer, actLen, buf.get(), actLen);
        if (ret != 0) {
            return {GetErrorCode(EIO), 0};
        }
    }
    return {SUCCESS_CODE, static_cast<int64_t>(actLen)};
}

tuple<int, int64_t> StreamImpl::ReadCur(uint8_t* buffer, size_t buLen, int64_t length)
{
    if (!fp_) {
        LOGE("Stream may have been closed");
        return {GetErrorCode(EIO), 0};
    }

    FILE *filp = nullptr;
    filp = fp_.get();

    auto [state, buf, len, offsetResult] = GetReadArg(static_cast<size_t>(buLen), length, 0.0);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    return ReadImpl(buf, len, filp, buffer);
}

tuple<int, int64_t> StreamImpl::Read(uint8_t* buffer, size_t buLen, int64_t length, int64_t offset)
{
    if (!fp_) {
        LOGE("Stream may have been closed");
        return {GetErrorCode(EIO), 0};
    }

    FILE *filp = nullptr;
    filp = fp_.get();

    auto [state, buf, len, offsetResult] = GetReadArg(static_cast<size_t>(buLen), length, offset);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    if (offsetResult >= 0) {
        int result = fseek(filp, static_cast<long>(offsetResult), SEEK_SET);
        if (result < 0) {
            LOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", result);
            return {GetErrorCode(errno), 0};
        }
    }
    return ReadImpl(buf, len, filp, buffer);
}

tuple<int, int64_t> StreamImpl::WriteCur(void* buffer, int64_t length, const std::string& encode)
{
    FILE *filp = nullptr;
    filp = fp_.get();

    auto [state, buf, len, offsetResult] =
        CommonFunc::GetWriteArg(buffer, length, 0, encode);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    size_t writeLen = fwrite(buf, 1, len, filp);
    if ((writeLen == 0) && (writeLen != len)) {
        LOGE("Failed to fwrite stream");
        return {GetErrorCode(EIO), 0};
    }
    return {SUCCESS_CODE, static_cast<int64_t>(writeLen)};
}

tuple<int, int64_t> StreamImpl::Write(void* buffer, int64_t length, int64_t offset, const std::string& encode)
{
    FILE *filp = nullptr;
    filp = fp_.get();

    auto [state, buf, len, offsetResult] =
        CommonFunc::GetWriteArg(buffer, length, offset, encode);
    if (state != SUCCESS_CODE) {
        LOGE("Failed to resolve buf and options");
        return {GetErrorCode(state), 0};
    }

    if (offsetResult >= 0) {
        int ret = fseek(filp, static_cast<long>(offsetResult), SEEK_SET);
        if (ret < 0) {
            LOGE("Failed to set the offset location of the file stream pointer, ret: %{public}d", ret);
            return {GetErrorCode(errno), 0};
        }
    }

    size_t writeLen = fwrite(buf, 1, len, filp);
    if ((writeLen == 0) && (writeLen != len)) {
        LOGE("Failed to fwrite stream");
        return {GetErrorCode(EIO), 0};
    }
    return {SUCCESS_CODE, static_cast<int64_t>(writeLen)};
}

}
}
}