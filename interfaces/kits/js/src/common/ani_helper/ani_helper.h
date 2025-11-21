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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_HELPER_H
#define INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_HELPER_H

#include <optional>
#include <string>
#include <tuple>

#include <ani.h>

#include "ani_cache.h"
#include "ani_signature.h"
#include "event_handler.h"
#include "event_runner.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

class AniHelper {
public:
    template <typename T>
    static ani_status SetFieldValue(ani_env *env, ani_object &obj, const string_view &key, const T &value)
    {
        ani_status status = ANI_ERROR;
        if constexpr (is_same_v<T, int> || is_same_v<T, int32_t> || is_same_v<T, ani_int>) {
            status = env->Object_SetFieldByName_Int(obj, key.data(), static_cast<ani_int>(value));
        } else if constexpr (is_same_v<T, long> || is_same_v<T, int64_t> || is_same_v<T, ani_long>) {
            status = env->Object_SetFieldByName_Long(obj, key.data(), static_cast<ani_long>(value));
        } else if constexpr (is_same_v<T, double> || is_same_v<T, ani_double>) {
            status = env->Object_SetFieldByName_Double(obj, key.data(), static_cast<ani_double>(value));
        } else if constexpr (is_same_v<T, bool> || is_same_v<T, ani_boolean>) {
            status = env->Object_SetFieldByName_Boolean(obj, key.data(), static_cast<ani_boolean>(value));
        } else if constexpr (is_same_v<T, string> || is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_SetFieldByName_Ref(obj, key.data(), move(aniStr));
        } else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<__ani_ref, std::remove_pointer_t<T>>) {
            status = env->Object_SetFieldByName_Ref(obj, key.data(), value);
        } else {
            HILOGE("Invalid ani value type!");
            return ANI_INVALID_TYPE;
        }
        return status;
    }

    template <typename T>
    static ani_status SetPropertyValue(ani_env *env, ani_object &obj, const string_view &key, const T &value)
    {
        ani_status status = ANI_ERROR;
        if constexpr (is_same_v<T, int> || is_same_v<T, int32_t> || is_same_v<T, ani_int>) {
            status = env->Object_SetPropertyByName_Int(obj, key.data(), static_cast<ani_int>(value));
        } else if constexpr (is_same_v<T, long> || is_same_v<T, int64_t> || is_same_v<T, ani_long>) {
            status = env->Object_SetPropertyByName_Long(obj, key.data(), static_cast<ani_long>(value));
        } else if constexpr (is_same_v<T, double> || is_same_v<T, ani_double>) {
            status = env->Object_SetPropertyByName_Double(obj, key.data(), static_cast<ani_double>(value));
        } else if constexpr (is_same_v<T, bool> || is_same_v<T, ani_boolean>) {
            status = env->Object_SetPropertyByName_Boolean(obj, key.data(), static_cast<ani_boolean>(value));
        } else if constexpr (is_same_v<T, string> || is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_SetPropertyByName_Ref(obj, key.data(), move(aniStr));
        } else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<__ani_ref, std::remove_pointer_t<T>>) {
            status = env->Object_SetPropertyByName_Ref(obj, key.data(), value);
        } else {
            HILOGE("Invalid ani value type!");
            return ANI_INVALID_TYPE;
        }
        return status;
    }

    static tuple<bool, optional<int64_t>> ParseInt64Option(ani_env *env, ani_object obj, const string &tag)
    {
        ani_boolean isUndefined = true;
        ani_ref property;
        ani_status status = ANI_ERROR;
        status = env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &property);
        if (status != ANI_OK) {
            return { false, nullopt };
        }
        env->Reference_IsUndefined(property, &isUndefined);
        if (isUndefined) {
            return { true, nullopt };
        }
        static const string longValueSig = Builder::BuildSignatureDescriptor({}, BasicTypes::longType);
        ani_long value;
        status = env->Object_CallMethodByName_Long(
            static_cast<ani_object>(property), BasicTypesConverter::toLong.c_str(), longValueSig.c_str(), &value);
        if (status != ANI_OK) {
            return { false, nullopt };
        }
        auto result = make_optional<int64_t>(static_cast<int64_t>(value));
        return { true, move(result) };
    }

    static tuple<bool, optional<int64_t>> ParseInt64Option(ani_env *env, ani_object obj, const string &className,
            const string &propertyName)
    {
        auto &aniCache = AniCache::GetInstance();
        auto [ret, method] =
            aniCache.GetMethod(env, className, propertyName, BoxedTypes::Long::getOptionSig);
        if (ANI_OK != ret) {
            return { false, nullopt };
        }
        ani_ref property;
        ret = env->Object_CallMethod_Ref(obj, method, &property);
        if (ret != ANI_OK) {
            return { false, nullopt };
        }
        ani_boolean isUndefined = true;
        env->Reference_IsUndefined(property, &isUndefined);
        if (isUndefined) {
            return { true, nullopt };
        }
        ani_long value{};
        tie(ret, method) = aniCache.GetMethod(env, BoxedTypes::Long::classDesc, BoxedTypes::Long::toLongDesc,
            BoxedTypes::Long::toLongSig);
        if (ANI_OK != ret) {
            return { false, nullopt };
        }

        ret = env->Object_CallMethod_Long(static_cast<ani_object>(property), method, &value);
        if (ANI_OK != ret) {
            HILOGE("Failed to Object_CallMethod_Long ret: %{public}d", ret);
            return { false, nullopt };
        }
        return { true, value };
    }

    static tuple<bool, optional<string>> ParseEncoding(ani_env *env, ani_object obj)
    {
        ani_boolean isUndefined;
        ani_ref property;
        if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, "encoding", &property)) {
            return { false, nullopt };
        }
        env->Reference_IsUndefined(property, &isUndefined);
        if (isUndefined) {
            return { true, nullopt };
        }
        auto [succ, encoding] = TypeConverter::ToUTF8String(env, (ani_string)property);
        if (!succ) {
            return { false, nullopt };
        }
        return { true, make_optional<string>(move(encoding)) };
    }

    static ani_env *&GetThreadEnvStorage()
    {
        static thread_local ani_env *env { nullptr };
        return env;
    }

    static ani_env *GetThreadEnv(ani_vm *vm)
    {
        auto &env = GetThreadEnvStorage();
        if (env != nullptr) {
            return env;
        }

        ani_options aniArgs { 0, nullptr };
        auto status = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env);
        if (status != ANI_OK) {
            status = vm->GetEnv(ANI_VERSION_1, &env);
            if (status != ANI_OK) {
                HILOGE("vm GetEnv, err: %{public}d", status);
                return nullptr;
            }
        }
        return env;
    }

    static void DetachThreadEnv(ani_vm *vm)
    {
        if (vm && GetThreadEnvStorage()) {
            auto status = vm->DetachCurrentThread();
            if (status != ANI_OK) {
                HILOGE("Detach thread env from vm failed! status: %{public}d", status);
                return;
            }
            GetThreadEnvStorage() = nullptr;
        }
    }

    static bool SendEventToMainThread(const function<void()> &func)
    {
        if (func == nullptr) {
            HILOGE("func is nullptr!");
            return false;
        }

        auto &mainHandler = GetMainHandler();
        if (!mainHandler) {
            HILOGE("mainHandler is not initialized!");
            return false;
        }
        bool succ = mainHandler->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::HIGH, {});
        if (!succ) {
            HILOGE("Failed to post task to main thread.");
            return false;
        }
        return true;
    }

private:
    static std::shared_ptr<OHOS::AppExecFwk::EventHandler> &GetMainHandler()
    {
        static std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler =
            []() -> std::shared_ptr<OHOS::AppExecFwk::EventHandler> {
            auto runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
            if (runner) {
                return CreateSharedPtr<OHOS::AppExecFwk::EventHandler>(runner);
            } else {
                HILOGE("Get main event runner failed when initializing mainHandler!");
                return nullptr;
            }
        }();

        return mainHandler;
    }
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_HELPER_H