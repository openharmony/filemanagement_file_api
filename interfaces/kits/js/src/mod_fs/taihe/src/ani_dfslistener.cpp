/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "ani_dfslistener.h"

#include <string>

#include "filemgmt_libhilog.h"
#include "taihe/runtime.hpp"
#include "taihe/platform/ani.hpp"
#include "fs_error.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

static bool CreateInt(ani_env *env, int value, ani_object *obj)
{
    if (env == nullptr || obj == nullptr) {
        HILOGE("CreateInt err, env is null: %{public}d, obj is null: %{public}d",
            env == nullptr, obj == nullptr);
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    static constexpr const char *className = "std.core.Int";
    ani_class intCls {};
    ani_status aniRes = env->FindClass(className, &intCls);
    if (aniRes != ANI_OK) {
        HILOGE("FindClass err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    ani_method ctor {};
    if ((aniRes = env->Class_FindMethod(intCls, "<ctor>", "i:", &ctor)) != ANI_OK) {
        HILOGE("Class_FindMethod err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    if ((aniRes = env->Object_New(intCls, ctor, obj, static_cast<ani_int>(value))) != ANI_OK) {
        HILOGE("Object_New err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    return true;
}

AniDfsListener::~AniDfsListener()
{
    if (listenerRef_ == nullptr) {
        HILOGE("listenerRef_ is null");
        return;
    }
    ani_env* etsEnv = nullptr;
    ani_options aniArgs { 0, nullptr };
    if (vm_ == nullptr) {
        HILOGE("~AniDfsListener err, vm_ is null");
        return;
    }
    ani_status aniRes = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv);
    if (aniRes != ANI_OK) {
        HILOGE("~AniDfsListener AttachCurrentThread err, res: %{public}d", static_cast<int>(aniRes));
        return;
    }
    if (etsEnv != nullptr) {
        aniRes = etsEnv->GlobalReference_Delete(listenerRef_);
        if (aniRes != ANI_OK) {
            HILOGE("~AniDfsListener GlobalReference_Delete err, res: %{public}d",
                static_cast<int>(aniRes));
        }
        listenerRef_ = nullptr;
        aniRes = vm_->DetachCurrentThread();
        if (aniRes != ANI_OK) {
            HILOGE("~AniDfsListener DetachCurrentThread with del err, res: %{public}d",
                static_cast<int>(aniRes));
        }
        return;
    }
    aniRes = vm_->DetachCurrentThread();
    if (aniRes != ANI_OK) {
        HILOGE("~AniDfsListener DetachCurrentThread err, res: %{public}d", static_cast<int>(aniRes));
    }
}

bool AniDfsListener::PrepareData(const ani_env* env, ani_object aniListeners)
{
    ani_env *tmpEnv = const_cast<ani_env*>(env);
    if (tmpEnv == nullptr) {
        HILOGE("PrepareData err, tmpEnv is null");
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    ani_status aniRes = tmpEnv->GlobalReference_Create(static_cast<ani_ref>(aniListeners), &listenerRef_);
    if (aniRes != ANI_OK) {
        HILOGE("GlobalReference_Create err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(ModuleFileIO::CommonErrCode::E_PARAMS,
            ::taihe::string_view("GlobalReference_Create failed"));
        return false;
    }
    if ((aniRes = tmpEnv->GetVM(&vm_)) != ANI_OK) {
        HILOGE("GetVM err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(ModuleFileIO::CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    if ((aniRes = tmpEnv->Object_GetPropertyByName_Ref(static_cast<ani_object>(listenerRef_),
        "onStatus", &onStatusRef_)) != ANI_OK) {
        HILOGE("Object_GetPropertyByName_Ref err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(ModuleFileIO::CommonErrCode::E_PARAMS,
            ::taihe::string_view("param error"));
        return false;
    }
    ani_boolean typeRes = ANI_FALSE;
    if ((aniRes = tmpEnv->Reference_IsNull(onStatusRef_, &typeRes)) != ANI_OK || typeRes) {
        HILOGE("Reference_IsNull err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    if ((aniRes = tmpEnv->Reference_IsUndefined(onStatusRef_, &typeRes)) != ANI_OK || typeRes) {
        HILOGE("Reference_IsUndefined err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return false;
    }
    return true;
}

static void DetachAniThread(ani_vm *vm)
{
    if (vm == nullptr) {
        HILOGE("DetachAniThread err, vm_ is null");
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    ani_status aniRes = vm->DetachCurrentThread();
    if (aniRes != ANI_OK) {
        HILOGE("DetachCurrentThread err, res: %{public}d", static_cast<int>(aniRes));
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
    }
}

void AniDfsListener::OnStatus(const std::string &networkId, int32_t status, const std::string &path, int32_t type)
{
    HILOGI("AniDfsListener::OnStatus start");
    ani_env* etsEnv = nullptr;
    ani_options aniArgs { 0, nullptr };
    if (vm_ == nullptr) {
        HILOGE("OnStatus err, vm_ is null");
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    ani_status aniRes = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &etsEnv);
    if (aniRes != ANI_OK || etsEnv == nullptr) {
        HILOGE("OnStatus AttachCurrentThread err, res: %{public}d, etsEnv == nullptr: %{public}d",
            static_cast<int>(aniRes), etsEnv == nullptr);
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    ani_ref fnReturnVal;
    std::vector<ani_ref> paramVec;
    ani_string aniNetworkId{};
    if ((aniRes = etsEnv->String_NewUTF8(networkId.c_str(), networkId.size(), &aniNetworkId)) != ANI_OK) {
        HILOGE("String_NewUTF8 err, res: %{public}d", static_cast<int>(aniRes));
        DetachAniThread(vm_);
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    ani_object aniStatusObj;
    if (!CreateInt(etsEnv, status, &aniStatusObj)) {
        DetachAniThread(vm_);
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    paramVec.push_back(static_cast<ani_ref>(aniNetworkId));
    paramVec.push_back(static_cast<ani_ref>(aniStatusObj));
    if ((aniRes = etsEnv->FunctionalObject_Call(static_cast<ani_fn_object>(onStatusRef_),
        paramVec.size(), paramVec.data(), &fnReturnVal)) != ANI_OK) {
        HILOGE("FunctionalObject_Call err, res: %{public}d", static_cast<int>(aniRes));
        DetachAniThread(vm_);
        ::taihe::set_business_error(CommonErrCode::E_PARAMS, ::taihe::string_view("param error"));
        return;
    }
    DetachAniThread(vm_);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS