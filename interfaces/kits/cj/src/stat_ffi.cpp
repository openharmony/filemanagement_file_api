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

#include "stat_ffi.h"
#include "stat_impl.h"
#include "macro.h"

using namespace OHOS::FFI;
using namespace OHOS::CJSystemapi;
using namespace OHOS::CJSystemapi::FileFs;

extern "C" {
int64_t FfiOHOSStatGetIno(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetIno");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetIno();
}
int64_t FfiOHOSStatGetMode(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetMode");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetMode();
}
int64_t FfiOHOSStatGetUid(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetUid");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetUid();
}
int64_t FfiOHOSStatGetGid(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetGid");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetGid();
}
int64_t FfiOHOSStatGetSize(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetSize");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetSize();
}
int64_t FfiOHOSStatGetAtime(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetAtime");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetAtime();
}
int64_t FfiOHOSStatGetMtime(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetMtime");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetMtime();
}
int64_t FfiOHOSStatGetCtime(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatGetCtime");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetCtime();
}

bool FfiOHOSStatIsBlockDevice(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsBlockDevice");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsBlockDevice();
}

bool FfiOHOSStatIsCharacterDevice(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsCharacterDevice");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsCharacterDevice();
}

bool FfiOHOSStatIsDirectory(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsDirectory");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsDirectory();
}

bool FfiOHOSStatIsFIFO(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsFIFO");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsFIFO();
}

bool FfiOHOSStatIsFile(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsFile");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsFile();
}

bool FfiOHOSStatIsSocket(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsSocket");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsSocket();
}

bool FfiOHOSStatIsSymbolicLink(int64_t id)
{
    LOGI("FS_TEST:: FfiOHOSStatIsSymbolicLink");
    auto instance = FFIData::GetData<StatImpl>(id);
    if (!instance) {
        LOGE("StatImpl instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->IsSymbolicLink();
}
}