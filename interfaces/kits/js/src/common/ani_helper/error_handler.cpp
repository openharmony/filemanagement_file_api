/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "error_handler.h"

#include "ani_helper.h"
#include "ani_signature.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

void ErrorHandler::Throw(
    ani_env *env, int32_t code, const std::string &errMsg, const std::optional<ani_object> &errData)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return;
    }

    auto err = CreateErrorObj(env, code, errMsg, errData);
    if (err == nullptr) {
        HILOGE("Create error object failed");
        return;
    }

    ani_status status = env->ThrowError(err);
    if (status != ANI_OK) {
        HILOGE("Throw ani error object failed, status: %{public}d", status);
        return;
    }
}

void ErrorHandler::Throw(ani_env *env, int32_t code, const std::optional<ani_object> &errData)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return;
    }
    FsError err(code);
    Throw(env, std::move(err), errData);
}

void ErrorHandler::Throw(ani_env *env, const FsError &err, const std::optional<ani_object> &errData)
{
    if (env == nullptr) {
        HILOGE("Invalid parameter env");
        return;
    }
    auto code = err.GetErrNo();
    const auto &errMsg = err.GetErrMsg();
    Throw(env, code, errMsg, errData);
}

ani_error ErrorHandler::CreateErrorObj(
    ani_env *env, int32_t code, const std::string &errMsg, const std::optional<ani_object> &errData)
{
    auto classDesc = BuiltInTypes::BusinessError::classDesc.c_str();
    AniCache &aniCache = AniCache::GetInstance();
    auto [ret, cls] = aniCache.GetClass(env, classDesc);
    if (ret != ANI_OK) {
        HILOGE("Get class failed, status: %{public}d", ret);
        return nullptr;
    }

    ani_method ctor;
    tie(ret, ctor) = aniCache.GetMethod(env, classDesc, BaseType::ctorDesc, BaseType::ctorSig0);
    if (ret != ANI_OK) {
        HILOGE("Get ctor method failed, status: %{public}d", ret);
        return nullptr;
    }

    ani_object obj;
    ani_status status = env->Object_New(cls, ctor, &obj);
    if (status != ANI_OK) {
        HILOGE("Cannot create ani error object, status: %{public}d", status);
        return nullptr;
    }

    status = AniHelper::SetPropertyValue(env, obj, "message", errMsg);
    if (status != ANI_OK) {
        HILOGE("Set property 'message' value failed, status: %{public}d", status);
        return nullptr;
    }

    status = AniHelper::SetPropertyValue(env, obj, "code", code);
    if (status != ANI_OK) {
        HILOGE("Set property 'code' value failed, status: %{public}d", status);
        return nullptr;
    }

    if (errData.has_value()) {
        status = AniHelper::SetPropertyValue(env, obj, "data", errData.value());
        if (status != ANI_OK) {
            HILOGE("Set property 'data' value failed, status: %{public}d", status);
            return nullptr;
        }
    }

    return static_cast<ani_error>(obj);
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
