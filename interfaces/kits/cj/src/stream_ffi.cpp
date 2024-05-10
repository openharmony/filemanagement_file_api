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

#include "stream_ffi.h"

using namespace OHOS::FFI;
using namespace OHOS::CJSystemapi;
using namespace OHOS::CJSystemapi::FileFs;

extern "C" {
RetCode FfiOHOSStreamClose(int64_t id)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamClose");
    auto instance = FFIData::GetData<StreamImpl>(id);
    return instance->Close();
}

RetCode FfiOHOSStreamFlush(int64_t id)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamFlush");
    auto instance = FFIData::GetData<StreamImpl>(id);
    return instance->Flush();
}

RetDataI64 FfiOHOSStreamWriteCur(int64_t id, const char* buffer, int64_t length, const char* encode)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamWriteCur");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<StreamImpl>(id);
    auto [state, writeLen] = instance->WriteCur(buffer, length, encode);
    ret.code = state;
    if (state != SUCCESS_CODE) {
        ret.data = 0;
        return ret;
    }
    ret.data = writeLen;
    return ret;
}

RetDataI64 FfiOHOSStreamWrite(int64_t id, const char* buffer, int64_t length, int64_t offset, const char* encode)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamWriteByString");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<StreamImpl>(id);
    auto [state, writeLen] = instance->Write(buffer, length, offset, encode);
    ret.code = state;
    if (state != SUCCESS_CODE) {
        ret.data = 0;
        return ret;
    }
    ret.data = writeLen;
    return ret;
}

RetDataI64 FfiOHOSStreamReadCur(int64_t id, uint8_t* buffer, int64_t bufLen, int64_t length)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamReadCur");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<StreamImpl>(id);
    auto [state, readLen] = instance->ReadCur(buffer, bufLen, length);
    ret.code = state;
    if (state != SUCCESS_CODE) {
        ret.data = 0;
        return ret;
    }
    ret.data = readLen;
    LOGI("OHOS::CJSystemapi FfiOHOSStreamReadCur success");
    return ret;
}

RetDataI64 FfiOHOSStreamRead(int64_t id, uint8_t* buffer, int64_t bufLen, int64_t length, int64_t offset)
{
    LOGI("OHOS::CJSystemapi FfiOHOSStreamRead");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<StreamImpl>(id);
    auto [state, readLen] = instance->Read(buffer, bufLen, length, offset);
    ret.code = state;
    if (state != SUCCESS_CODE) {
        ret.data = 0;
        return ret;
    }
    ret.data = readLen;
    LOGI("OHOS::CJSystemapi FfiOHOSStreamRead success");
    return ret;
}
}