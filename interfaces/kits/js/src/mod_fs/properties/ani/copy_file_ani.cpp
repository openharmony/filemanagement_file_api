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

#include "copy_file_ani.h"

#include "copy_file_core.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

void CopyFileAni::CopyFileSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_object src, ani_object dest, ani_object mode)
{
    auto [succSrc, srcFile] = TypeConverter::ToFileInfo(env, src);
    auto [succDest, destFile] = TypeConverter::ToFileInfo(env, dest);
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath/fd");
        return;
    }

    auto [succMode, optMode] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        return;
    }

    auto ret = CopyFileCore::DoCopyFile(srcFile, destFile, optMode);
    if (!ret.IsSuccess()) {
        HILOGE("DoCopyFile failed!");
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS