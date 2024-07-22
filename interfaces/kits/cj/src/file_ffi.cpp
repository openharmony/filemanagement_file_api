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

#include "file_impl.h"
#include "file_fs_impl.h"
#include "file_ffi.h"
#include "macro.h"
#include "uni_error.h"

using namespace OHOS::FFI;
using namespace OHOS::CJSystemapi;
namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

extern "C" {
RetDataI64 FfiOHOSFileFsOpen(const char* path, int64_t mode)
{
    LOGI("FS_TEST::FfiOHOSFILEOpen");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStream] = FileEntity::Open(path, mode);
    if (state != SUCCESS_CODE) {
        LOGE("FS_TEST::FfiOHOSFILEOpen error");
        ret.code = GetErrorCode(state);
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFILEOpen success");
    ret.code = state;
    ret.data = nativeStream->GetID();
    return ret;
}

int FfiOHOSFileFsCloseByFd(int32_t file)
{
    LOGI("FS_TEST::FfiOHOSFileFsClose");
    int err = FileFsImpl::Close(file);
    LOGI("FS_TEST::FfiOHOSFileFsClose success");
    return err;
}

int FfiOHOSFileFsClose(int64_t file)
{
    LOGI("FS_TEST::FfiOHOSFileFsClose");
    auto instance = FFIData::GetData<FileEntity>(file);
    if (!instance) {
        LOGE("Stream instance not exist %{public}" PRId64, file);
        return ERR_INVALID_INSTANCE_CODE;
    }
    int err = FileFsImpl::Close(instance);
    FFIData::Release(file);
    LOGI("FS_TEST::FfiOHOSFileFsClose success");
    return err;
}

RetDataI64 FfiOHOSFileFsDup(int32_t fd)
{
    LOGI("FS_TEST::FfiOHOSFileFsDup");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeFile] = FileEntity::Dup(fd);
    if (state != SUCCESS_CODE) {
        LOGE("FS_TEST::FfiOHOSFileFsDup error");
        ret.code = GetErrorCode(state);
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsDup success");
    ret.code = state;
    ret.data = nativeFile->GetID();
    return ret;
}

int FfiOHOSFILEFsGetFD(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFILEGetFD");
    auto instance = FFIData::GetData<FileEntity>(id);
    if (!instance) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->GetFD(id);
}

const char* FfiOHOSFILEFsGetPath(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFILEGetPath");
    auto instance = FFIData::GetData<FileEntity>(id);
    if (!instance) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return "error";
    }
    return instance->GetPath(id);
}

const char* FfiOHOSFILEFsGetName(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFILEGetName");
    auto instance = FFIData::GetData<FileEntity>(id);
    if (!instance) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return "error";
    }
    return instance->GetName(id);
}

RetCode FfiOHOSFILEFsTryLock(int64_t id, bool exclusive)
{
    LOGI("FS_TEST::FfiOHOSFILEFsTryLock");
    auto instance = FFIData::GetData<FileEntity>(id);
    if (!instance) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->TryLock(id, exclusive);
}

RetCode FfiOHOSFILEFsUnLock(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFILEFsUnLock");
    auto instance = FFIData::GetData<FileEntity>(id);
    if (!instance) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return instance->UnLock(id);
}

RetDataCString FfiOHOSFILEFsGetParent(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFILEFsGetParent");
    auto instance = FFIData::GetData<FileEntity>(id);
    RetDataCString ret = { .code = EINVAL, .data = nullptr };
    if (!instance) {
        LOGE("FS_TEST::FfiOHOSFILEFsGetParent instance not exist %{public}" PRId64, id);
        return ret;
    }
    ret = instance->GetParent();
    if (ret.code != SUCCESS_CODE) {
        ret.code = GetErrorCode(ret.code);
    }
    LOGI("FS_TEST::FfiOHOSFILEFsGetParent end");
    return ret;
}
}
} // namespace FileFs
} // namespace CJSystemapi
} // namespace OHOS