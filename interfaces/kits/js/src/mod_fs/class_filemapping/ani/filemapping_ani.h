/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_ANI_FILEMAPPING_ANI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_ANI_FILEMAPPING_ANI_H

#include <ani.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

class FileMappingAni final {
public:
    static void SetPosition(ani_env *env, ani_object object, ani_int position);
    static ani_int GetPosition(ani_env *env, ani_object object);
    static ani_int Capacity(ani_env *env, ani_object object);
    static void SetLimit(ani_env *env, ani_object object, ani_int limit);
    static ani_int GetLimit(ani_env *env, ani_object object);
    static void Flip(ani_env *env, ani_object object);
    static ani_int Remaining(ani_env *env, ani_object object);
    static ani_int Read(ani_env *env, ani_object object,
        ani_arraybuffer buffer, ani_object length);
    static ani_int ReadFrom(ani_env *env, ani_object object, ani_int position,
        ani_arraybuffer buffer, ani_object length);
    static ani_int Write(ani_env *env, ani_object object,
        ani_arraybuffer data, ani_object length);
    static ani_int WriteTo(ani_env *env, ani_object object, ani_int position,
        ani_arraybuffer data, ani_object length);
    static void MsyncSync(ani_env *env, ani_object object);
    static void MsyncSyncWith(ani_env *env, ani_object object, ani_int position, ani_int length);
    static void UnmapSync(ani_env *env, ani_object object);
};

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_ANI_FILEMAPPING_ANI_H
