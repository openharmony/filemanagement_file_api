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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ANI_DISCONNECTDFS_ANI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ANI_DISCONNECTDFS_ANI_H

#include <ani.h>
#include "dfs_listener/file_dfs_listener_stub.h"
#include "distributed_file_daemon_manager.h"
#include "filemgmt_libfs.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

class DisConnectDfsAni final {
public:
    static void DisConnectDfsSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string networkId);
};

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_ANI_DISCONNECTDFS_ANI_H
