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

#include "disconnectdfs_ani.h"
#include "disconnectdfs_core.h"
#include "filemgmt_libhilog.h"
#include "error_handler.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
void DisConnectDfsAni::DisConnectDfsSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string networkId)
{
    auto [succNetworkId, networkIdStr] = TypeConverter::ToUTF8String(env, networkId);
    if (!succNetworkId) {
        HILOGE("Invalid NetworkId");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
    auto ret = DisConnectDfsCore::DisConnectDfsExec(networkIdStr);
    if (!ret.IsSuccess()) {
        HILOGE("DisConnectDfsExec failed");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS