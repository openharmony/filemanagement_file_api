/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "error_handler.h"
#include "file_wrapper.h"
#include "filemgmt_libhilog.h"
#include "mmap_core.h"
#include "type_converter.h"
#include "filemapping_wrapper.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

static bool ValidateMmapParams(ani_env *env, int fd, int mode, ani_long offset, ani_int size)
{
    if (fd < 0) {
        HILOGE("Invalid fd value");
        ErrorHandler::Throw(env, EBADF);
        return false;
    }
    if (mode < 0 || mode > MappingMode::PRIVATE) {
        HILOGE("Invalid mode value");
        ErrorHandler::Throw(env, EINVAL);
        return false;
    }
    if (offset < 0) {
        HILOGE("Invalid offset value");
        ErrorHandler::Throw(env, EINVAL);
        return false;
    }
    if (size <= 0) {
        HILOGE("Invalid size value");
        ErrorHandler::Throw(env, EINVAL);
        return false;
    }
    return true;
}

ani_object MmapAni::MmapSync(ani_env *env, [[maybe_unused]] ani_class clazz,
    ani_object file, ani_enum_item mode,
    ani_long offset, ani_int size)
{
#ifdef FILE_API_TRACE
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
#endif

    auto [succPathOrFd, fileInfo] = TypeConverter::ToFileInfo(env, file);
    if (!succPathOrFd) {
        HILOGE("Invalid file parameter");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    int fd = -1;
    if (fileInfo.isPath) {
        HILOGE("mmap does not support path, need fd or File object");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    } else {
        if (!fileInfo.fdg) {
            HILOGE("Invalid fd");
            ErrorHandler::Throw(env, EBADF);
            return nullptr;
        }
        fd = fileInfo.fdg->GetFD();
    }

    auto [succMode, modeOp] = TypeConverter::EnumToInt32(env, mode);
    if (!succMode || !modeOp.has_value()) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    int modeValue = modeOp.value();

    if (!ValidateMmapParams(env, fd, modeValue, offset, size)) {
        return nullptr;
    }

    auto result = MmapCore::DoMmap(fd, modeValue, static_cast<off_t>(offset), static_cast<size_t>(size));
    if (!result.IsSuccess()) {
        HILOGE("DoMmap failed");
        const auto &err = result.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    FsFileMapping *mapping = result.GetData().value();
    auto wrapResult = FileMappingWrapper::Wrap(env, mapping);
    if (wrapResult == nullptr) {
        delete mapping;
        mapping = nullptr;
        ErrorHandler::Throw(env, FILEIO_SYS_CAP_TAG + E_INTERN_RES);
        return nullptr;
    }
    return wrapResult;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
