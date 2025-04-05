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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FDOPEN_STREAM_ANI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FDOPEN_STREAM_ANI_H

#include <ani.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
class FdopenStreamAni final {
public:
    static ani_object FdopenStreamSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_double fd, ani_string mode);
};
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_FDOPEN_STREAM_ANI_H