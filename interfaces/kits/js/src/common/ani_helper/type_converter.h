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

#ifndef FILEMANAGEMENT_ANI_TYPE_CONVERTER_H
#define FILEMANAGEMENT_ANI_TYPE_CONVERTER_H

#include <string>
#include <tuple>

#include <ani.h>

#include "fs_array_buffer.h"
#include "fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
inline const std::string EMPTY_STRING = "";

class TypeConverter {
public:
    static std::tuple<bool, std::string> ToUTF8String(ani_env *env, const ani_string &path);
    static std::tuple<bool, std::optional<int32_t>> ToOptionalInt32(ani_env *env, const ani_object &value);
    static std::tuple<bool, std::optional<int64_t>> ToOptionalInt64(ani_env *env, const ani_object &value);
    static std::tuple<bool, ani_arraybuffer> ToAniArrayBuffer(ani_env *env, void *buffer, size_t length);
    static std::tuple<bool, ani_string> ToAniString(ani_env *env, std::string str);
    static std::tuple<bool, ani_string> ToAniString(ani_env *env, std::string str, size_t size);
    static std::tuple<bool, ani_string> ToAniString(ani_env *env, const char *str);
    static std::tuple<bool, std::optional<int32_t>> EnumToInt32(ani_env *env, const ani_enum_item &enumOp);
    static std::tuple<bool, FileInfo> ToFileInfo(ani_env *env, const ani_object &pathOrFd);
    static std::tuple<bool, ArrayBuffer> ToArrayBuffer(ani_env *env, ani_arraybuffer &buffer);
    static std::tuple<bool, ani_array> ToAniStringList(
        ani_env *env, const std::string strList[], const uint32_t length);
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI

#endif // FILEMANAGEMENT_ANI_TYPE_CONVERTER_H
