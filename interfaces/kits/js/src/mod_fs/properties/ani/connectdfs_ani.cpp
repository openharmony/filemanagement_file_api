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

#include "connectdfs_ani.h"
#include "connectdfs_core.h"
#include "filemgmt_libhilog.h"
#include "error_handler.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
void ConnectDfsAni::ConnectDfsSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string networkId,
                                   ani_object listener)
{
    //注册
    auto [succNetworkId, networkIdStr] = TypeConverter::ToUTF8String(env, networkId);
    if (!succNetworkId) {
        HILOGE("Invalid NetworkId");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
    sptr<ANIDfsListener> anidfslistener_(new(std::nothrow) ANIDfsListener());
    anidfslistener_->SetConnectDfsEnv(env);
    ani_ref promiseDeferred = nullptr;

    if (listener != nullptr) {
        anidfslistener_->SetConnectDfsCBRef(listener);
        HILOGE("Connectdfs set callback success");
    } else {
        anidfslistener_->SetConnectDfsPromiseRef(promiseDeferred);
        HILOGE("Connectdfs set Promise success");
    }

    auto ret = ConnectDfsCore::ConnectDfsExec(networkIdStr, anidfslistener_);
    if (!ret.IsSuccess()) {
        HILOGE("ConnectDfsExec failed");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
}

void ANIDfsListener::SetConnectDfsEnv(ani_env *env)
{
    env_ = env;
}

void ANIDfsListener::SetConnectDfsCBRef(ani_ref ref)
{
    onStatusRef_ = ref;
}

void ANIDfsListener::SetConnectDfsPromiseRef(ani_ref promiseDeferred)
{
    promiseDeferred_ = promiseDeferred;
}

void ANIDfsListener::OnStatus(const std::string &networkId, int32_t status)
{
    //回调
    ani_string networkId_;
    ani_status res = env_->String_NewUTF8(networkId.c_str(), networkId.size(), &networkId_);
    if (res != ANI_OK) {
        HILOGE("std string to ani string failed. ret = %{public}d", static_cast<int32_t>(res));
        ErrorHandler::Throw(env_, E_PARAMS);
        return;
    }
    ani_double status_ = static_cast<ani_double>(status);
    ani_status ret = env_->Object_CallMethodByName_Void(static_cast<ani_object>(objectlistener), "onStatus", nullptr,
                                                        networkId_, status_);
    if (ret != ANI_OK) {
        HILOGE("Object_CallMethodByName_Void failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env_, E_PARAMS);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS