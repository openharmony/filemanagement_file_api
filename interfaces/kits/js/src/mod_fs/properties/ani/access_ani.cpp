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

#include "access_ani.h"

#include <string>

#include "access_core.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

static constexpr int EXIST_INDEX = 0;
static constexpr int WRITE_INDEX = 2;
static constexpr int READ_INDEX = 4;
static constexpr int READ_WRITE_INDEX = 6;

static constexpr int LOCAL_INDEX = 0;

AccessModeType ToAccessModeType(int32_t mode_index)
{
    switch (mode_index) {
        case EXIST_INDEX:
            return AccessModeType::EXIST;
        case WRITE_INDEX:
            return AccessModeType::WRITE;
        case READ_INDEX:
            return AccessModeType::READ;
        case READ_WRITE_INDEX:
            return AccessModeType::READ_WRITE;
        default:
            return AccessModeType::ERROR;
    }
}

std::optional<AccessModeType> OptToAccessModeType(const std::optional<int>& mode_index)
{
    if (!mode_index.has_value()) {
        return std::nullopt;
    }
    return ToAccessModeType(mode_index.value());
}

AccessFlagType ToAccessFlagType(int32_t flag_index)
{
    switch (flag_index) {
        case LOCAL_INDEX:
            return AccessFlagType::LOCAL_FLAG;
        default:
            return AccessFlagType::DEFAULT_FLAG;
    }
}

std::optional<AccessFlagType> OptToAccessFlagType(const std::optional<int> &flag_index)
{
    if (!flag_index.has_value()) {
        return std::nullopt;
    }
    return ToAccessFlagType(flag_index.value());
}

ani_boolean AccessAni::AccessSync3(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path,
    ani_enum_item mode, ani_enum_item flag)
{
    ani_boolean ret = 0;
    auto [succPath, pathStr] = TypeConverter::ToUTF8StringPath(env, path);
    if (! succPath) {
        HILOGE("Invalid path");
        return ret;
    }

    auto [succMode, modeOp] = TypeConverter::ToOptionalInt32(env, mode);
    if (! succMode) {
        HILOGE("Invalid mode");
        return ret;
    }
    auto modeType = OptToAccessModeType(modeOp);

    auto [succFlag, flagOpt] = TypeConverter::ToOptionalInt32(env, flag);
    if (! succFlag) {
        HILOGE("Invalid flag");
        return ret;
    }
    auto flagType = OptToAccessFlagType(flagOpt);

    FsResult<bool> fsRet = FsResult<bool>::Success(false);
    if (flagOpt == std::nullopt) {
        fsRet = AccessCore::DoAccess(pathStr, modeType);
    } else {
        fsRet = AccessCore::DoAccess(pathStr, modeType.value(), flagType.value());
    }

    if (!fsRet.IsSuccess()) {
        HILOGE("IsSuccess Fail");
        return false;
    }
    return fsRet.GetData().value();
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS