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

#include "file_impl.h"
#include "file_fs_impl.h"
#include "file_ffi.h"
#include "macro.h"
#include "uni_error.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "file_n_exporter.h"
#include "file_entity.h"
#include "n_class.h"
#include "n_val.h"

using namespace OHOS::FFI;
using namespace OHOS::CJSystemapi;
namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

extern "C" {
void RegisterAndSaveClass(napi_env env)
{
    std::vector<napi_property_descriptor> props = {
        FileManagement::LibN::NVal::DeclareNapiGetter("fd",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::GetFD),
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        FileManagement::LibN::NVal::DeclareNapiGetter("path",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::GetPath),
        FileManagement::LibN::NVal::DeclareNapiGetter("name",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::GetName),
        FileManagement::LibN::NVal::DeclareNapiFunction("lock",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::Lock),
        FileManagement::LibN::NVal::DeclareNapiFunction("tryLock",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::TryLock),
        FileManagement::LibN::NVal::DeclareNapiFunction("unlock",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::UnLock),
        FileManagement::LibN::NVal::DeclareNapiFunction("getParent",
            OHOS::FileManagement::ModuleFileIO::FileNExporter::GetParent),
#endif
    };

    bool succ = false;
    napi_value classValue = nullptr;
    std::tie(succ, classValue) = FileManagement::LibN::NClass::DefineClass(
        env,
        OHOS::FileManagement::ModuleFileIO::FileNExporter::className_,
        OHOS::FileManagement::ModuleFileIO::FileNExporter::Constructor,
        move(props));
    FileManagement::LibN::NClass::SaveClass(
        env,
        OHOS::FileManagement::ModuleFileIO::FileNExporter::className_,
        classValue);
}

napi_value FfiConvertFile2Napi(napi_env env, int64_t id)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    auto instance = FFIData::GetData<FileEntity>(id);

    if (instance == nullptr) {
        LOGE("FfiConvertFile2Napi instance not exist %{public}" PRId64, id);
        return undefined;
    }
    RegisterAndSaveClass(env);
    napi_value objRAF = FileManagement::LibN::NClass::InstantiateClass(
        env,
        OHOS::FileManagement::ModuleFileIO::FileNExporter::className_,
        {});
    if (!env || !objRAF) {
        LOGE("Empty input: env %d, obj %d", env == nullptr, objRAF == nullptr);
        return undefined;
    }

    OHOS::FileManagement::ModuleFileIO::FileEntity *rafEntity = nullptr;
    napi_status status = napi_unwrap(env, objRAF, reinterpret_cast<void **>(&rafEntity));
    if (status != napi_ok) {
        LOGE("Cannot unwrap for pointer: %d", status);
        return undefined;
    }
    if (!rafEntity) {
        LOGE("Cannot instantiate file because of void entity");
        return undefined;
    }

    rafEntity->fd_.swap(instance->fd_);
    rafEntity->path_ = instance->path_;
    rafEntity->uri_ = instance->uri_;

    napi_valuetype type;
    status = napi_typeof(env, objRAF, &type);
    if (status != napi_ok || type == napi_undefined) {
        LOGE("[File]: create napiobj failed");
        return undefined;
    }

    return objRAF;
}

int64_t FfiCreateFileFromNapi(napi_env env, napi_value objRAF)
{
    using ModuleFileIOFileEntity = OHOS::FileManagement::ModuleFileIO::FileEntity;
    using CJFileEntity = OHOS::CJSystemapi::FileFs::FileEntity;

    if (env == nullptr || objRAF == nullptr) {
        LOGE("[File]: parameter is nullptr");
        return ERR_INVALID_INSTANCE_CODE;
    }

    napi_valuetype type = napi_undefined;

    if (napi_typeof(env, objRAF, &type) != napi_ok || type != napi_object) {
        LOGE("[File]: parameter is not object");
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto fileEntity = FileManagement::LibN::NClass::GetEntityOf<ModuleFileIOFileEntity>(env, objRAF);
    if (!fileEntity) {
        LOGE("[File]: Cannot instantiate file because of void entity");
        return ERR_INVALID_INSTANCE_CODE;
    }

    auto native = FFIData::Create<CJFileEntity>(std::move(fileEntity->fd_), fileEntity->path_, fileEntity->uri_);
    if (native == nullptr) {
        LOGE("[File]: Create ffidata failed");
        return ERR_INVALID_INSTANCE_CODE;
    }

    return native->GetID();
}

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