/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mmap_ani.h"

#include "ani_helper.h"
#include "error_handler.h"
#include "file_fs_trace.h"
#include "file_wrapper.h"
#include "filemapping_wrapper.h"
#include "filemgmt_libhilog.h"
#include "mmap_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

static tuple<bool, int32_t, FsFile*> ParseFdOrFile(ani_env *env, ani_object obj)
{
    int32_t result = -1;
    AniCache& aniCache = AniCache::GetInstance();
    auto [ret, intClass] = aniCache.GetClass(env, BoxedTypes::Int::classDesc);
    if (ret != ANI_OK) {
        HILOGE("Failed to get Int class");
        return { false, result, nullptr };
    }

    ani_boolean isInt;
    if (ANI_OK != env->Object_InstanceOf(obj, intClass, &isInt)) {
        HILOGE("Failed to instance object");
        return { false, result, nullptr };
    }
    if (isInt) {
        ani_int fd;
        if (ANI_OK != env->Object_CallMethodByName_Int(obj, BasicTypesConverter::toInt.c_str(), nullptr, &fd)) {
            HILOGE("Get fd value failed");
            return { false, result, nullptr };
        }
        result = static_cast<int32_t>(fd);
        return { true, result, nullptr };
    }

    FsFile *file = FileWrapper::Unwrap(env, obj);
    if (file != nullptr) {
        return { true, -1, file };
    }

    HILOGE("Cannot unwrap fsfile!");
    ErrorHandler::Throw(env, FILEIO_SYS_CAP_TAG + E_INTERN_RES);
    return { false, -1, nullptr };
}

static tuple<bool, int, off_t, size_t> ValidateMmapParams(ani_env *env,
    ani_enum_item mode, ani_long offset, ani_int size)
{
    auto [succMode, modeOp] = TypeConverter::EnumToInt32(env, mode);
    if (!succMode || !modeOp.has_value()) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return { false, 0, 0, 0 };
    }

    if (offset < 0) {
        HILOGE("Invalid offset value");
        ErrorHandler::Throw(env, EINVAL);
        return { false, 0, 0, 0 };
    }
    if (size <= 0) {
        HILOGE("Invalid size value");
        ErrorHandler::Throw(env, EINVAL);
        return { false, 0, 0, 0 };
    }

    return { true, modeOp.value(), static_cast<off_t>(offset), static_cast<size_t>(size) };
}

ani_object MmapAni::MmapSync(ani_env *env, [[maybe_unused]] ani_class clazz,
    ani_object file, ani_enum_item mode, ani_long offset, ani_int size)
{
    FileFsTrace traceMmap("MmapSync");

    auto [succ, fd, fsFile] = ParseFdOrFile(env, file);
    if (!succ) {
        HILOGE("Parse fd or file argument failed");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    if (fsFile != nullptr) {
        auto res = fsFile->GetFD();
        if (!res.IsSuccess()) {
            HILOGE("Parse file argument failed");
            ErrorHandler::Throw(env, EINVAL);
            return nullptr;
        }
        fd = res.GetData().value();
    }

    auto [valid, modeValue, off, sz] = ValidateMmapParams(env, mode, offset, size);
    if (!valid) {
        return nullptr;
    }

    auto result = MmapCore::DoMmap(fd, modeValue, off, sz);
    if (!result.IsSuccess()) {
        HILOGE("DoMmap failed");
        ErrorHandler::Throw(env, result.GetError());
        return nullptr;
    }

    FsFileMapping *mapping = result.GetData().value();
    auto wrapResult = FileMappingWrapper::Wrap(env, mapping);
    if (wrapResult == nullptr) {
        HILOGE("Failed to wrap file mapping");
        delete mapping;
        ErrorHandler::Throw(env, FILEIO_SYS_CAP_TAG + E_INTERN_RES);
        return nullptr;
    }
    return wrapResult;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
