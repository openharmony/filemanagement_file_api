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

#include "ohos.file.fs.proj.hpp"
#include "ohos.file.fs.impl.hpp"
#include "stdexcept"

#include <string>

#include "ani_dfslistener.h"
#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"
#include "fs_error.h"
#include "taihe/runtime.hpp"

namespace {
using AniDfsListener = OHOS::FileManagement::ModuleFileIO::AniDfsListener;
void ConnectDfsSync(::taihe::string_view networkId, uintptr_t listeners)
{
    taihe::env_guard envGuard;
    ani_env* env = envGuard.get_env();
    ani_object ani_listeners = reinterpret_cast<ani_object>(listeners);
    if (env == nullptr || ani_listeners == nullptr) {
        HILOGE("env or ani_listeners is null, env: %{public}d, ani_listeners: %{public}d",
            env == nullptr, ani_listeners == nullptr);
        ::taihe::set_business_error(OHOS::FileManagement::ModuleFileIO::CommonErrCode::E_PARAMS,
            ::taihe::string_view("env or ani_listeners is null"));
        return;
    }
    OHOS::sptr<AniDfsListener> aniDfsListeners{new (std::nothrow) AniDfsListener()};
    if (!aniDfsListeners) {
        HILOGE("AniDfsListener is null");
        ::taihe::set_business_error(OHOS::FileManagement::ModuleFileIO::CommonErrCode::E_PARAMS,
            ::taihe::string_view("new AniDfsListener failed"));
        return;
    }
    if (!aniDfsListeners->PrepareData(env, ani_listeners)) {
        ::taihe::set_business_error(OHOS::FileManagement::ModuleFileIO::CommonErrCode::E_PARAMS,
            ::taihe::string_view("PrepareData failed"));
        return;
    }
    auto ret = OHOS::Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
        OpenP2PConnectionEx(std::string(networkId), aniDfsListeners);
    if (ret != OHOS::FileManagement::ModuleFileIO::ERRNO_NOERR) {
        HILOGE("ConnectDfsSync failed, ret: %{public}d", ret);
        OHOS::FileManagement::ModuleFileIO::FsError fsError(ret);
        ::taihe::set_business_error(fsError.GetErrNo(), ::taihe::string_view(fsError.GetErrMsg()));
    }
}

void DisconnectDfsSync(::taihe::string_view networkId)
{
    auto ret = OHOS::Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
        CloseP2PConnectionEx(std::string(networkId));
    if (ret != OHOS::FileManagement::ModuleFileIO::ERRNO_NOERR) {
        HILOGE("DisconnectDfsSync failed, ret: %{public}d", ret);
        OHOS::FileManagement::ModuleFileIO::FsError fsError(ret);
        ::taihe::set_business_error(fsError.GetErrNo(), ::taihe::string_view(fsError.GetErrMsg()));
    }
}
} // namespace

TH_EXPORT_CPP_API_ConnectDfsSync(ConnectDfsSync);
TH_EXPORT_CPP_API_DisconnectDfsSync(DisconnectDfsSync);
