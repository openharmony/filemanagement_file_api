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

#include "watch_event_listener.h"

#include <memory>
#include "ani_helper.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"
#include "watch_event_wrapper.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;

bool WatchEventListener::IsStrictEquals(const shared_ptr<IWatcherCallback> &other) const
{
    if (other->GetClassName() != className_) {
        return false;
    }

    const auto otherListener = static_pointer_cast<const WatchEventListener>(other);
    if (!otherListener) {
        HILOGE("Cannot convert IWatcherCallback to WatchEventListener");
        return false;
    }

    ani_env *env = AniHelper::GetThreadEnv(vm);
    if (!env) {
        HILOGE("Current object's env is null");
        return false;
    }

    ani_boolean isSame = false;
    ani_status status = env->Reference_StrictEquals(callback, otherListener->callback, &isSame);
    AniHelper::DetachThreadEnv(vm);
    if (status != ANI_OK) {
        HILOGE("Compare ref for strict equality failed. status = %{public}d", static_cast<int32_t>(status));
        return false;
    }
    return isSame;
}

void WatchEventListener::InvokeCallback(const string &fileName, uint32_t event, uint32_t cookie) const
{
    auto watchEvent = CreateSharedPtr<WatchEvent>();
    if (watchEvent == nullptr) {
        HILOGE("Failed to request heap memory.");
        return;
    }

    watchEvent->fileName = fileName;
    watchEvent->event = event;
    watchEvent->cookie = cookie;
    SendWatchEvent(*watchEvent);
    AniHelper::DetachThreadEnv(vm);
}

inline static const int32_t ANI_SCOPE_SIZE = 16;

void WatchEventListener::SendWatchEvent(const WatchEvent &watchEvent) const
{
    if (vm == nullptr) {
        HILOGE("Cannot send WatchEvent because the vm is null.");
        return;
    }
    if (callback == nullptr) {
        HILOGE("Cannot send WatchEvent because the callback is null.");
        return;
    }
    ani_size scopeSize = ANI_SCOPE_SIZE;
    ani_env *env = AniHelper::GetThreadEnv(vm);
    if (env == nullptr) {
        HILOGE("Cannot send WatchEvent because the env is null.");
        return;
    }
    ani_status status = env->CreateLocalScope(scopeSize);
    if (status != ANI_OK) {
        HILOGE("Failed to creat local scope, status: %{public}d", static_cast<int32_t>(status));
        return;
    }
    auto evtObj = WatchEventWrapper::Wrap(env, watchEvent);
    if (evtObj == nullptr) {
        HILOGE("Create WatchEvent obj failed!");
        return;
    }
    vector<ani_ref> args = { static_cast<ani_ref>(evtObj) };
    auto argc = args.size();
    ani_ref result;
    auto cbObj = static_cast<ani_fn_object>(callback);
    status = env->FunctionalObject_Call(cbObj, argc, args.data(), &result);
    if (status != ANI_OK) {
        HILOGE("Failed to call FunctionalObject_Call, status: %{public}d", static_cast<int32_t>(status));
        // continue execution and not exit.
    }
    status = env->DestroyLocalScope();
    if (status != ANI_OK) {
        HILOGE("Failed to destroy local scope, status: %{public}d", static_cast<int32_t>(status));
        return;
    }
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
