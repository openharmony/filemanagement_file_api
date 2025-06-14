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

#include "ani_signature.h"
#include "event_handler.h"
#include "event_runner.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

inline shared_ptr<OHOS::AppExecFwk::EventHandler>& GetMainHandler() {
    thread_local shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler;
    return mainHandler;
}

class AniHelper {
public:
    template <typename T>
    static ani_status SetFieldValue(
        ani_env *env, const ani_class &cls, ani_object &obj, const char *fieldName, const T &value)
    {
        ani_field field;
        auto status = env->Class_FindField(cls, fieldName, &field);
        if (status != ANI_OK) {
            return status;
        }

        if constexpr (is_same_v<T, int> || is_same_v<T, int32_t> || is_same_v<T, ani_int>) {
            status = env->Object_SetField_Int(obj, field, value);
        } else if constexpr (is_same_v<T, int64_t> || is_same_v<T, ani_long>) {
            status = env->Object_SetField_Long(obj, field, value);
        } else if constexpr (is_same_v<T, double> || is_same_v<T, ani_double>) {
            status = env->Object_SetField_Double(obj, field, value);
        } else if constexpr (is_same_v<T, bool> || is_same_v<T, ani_boolean>) {
            status = env->Object_SetField_Boolean(obj, field, value);
        } else if constexpr (is_same_v<T, string> || is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_SetField_Ref(obj, field, move(aniStr));
        } else if constexpr (is_base_of_v<ani_ref, T>) {
            status = env->Object_SetField_Ref(obj, field, value);
        } else {
            return ANI_INVALID_TYPE;
        }
        return status;
    }

    template <typename T>
    static ani_status SetPropertyValue(
        ani_env *env, const ani_class &cls, ani_object &obj, const string &property, const T &value)
    {
        ani_method method;
        string setter = "<set>" + property;
        auto status = env->Class_FindMethod(cls, setter.c_str(), nullptr, &method);
        if (status != ANI_OK) {
            return status;
        }

        if constexpr (is_same_v<T, string> || is_same_v<T, const char *>) {
            auto [succ, aniStr] = TypeConverter::ToAniString(env, value);
            if (!succ) {
                return ANI_ERROR;
            }
            status = env->Object_CallMethod_Void(obj, method, move(aniStr));
        } else if constexpr (is_base_of_v<ani_ref, T> || is_same_v<T, int> || is_same_v<T, int32_t> ||
                             is_same_v<T, ani_int> || is_same_v<T, int64_t> || is_same_v<T, ani_long> ||
                             is_same_v<T, double> || is_same_v<T, ani_double> || is_same_v<T, bool> ||
                             is_same_v<T, ani_boolean>) {
            status = env->Object_CallMethod_Void(obj, method, value);
        } else {
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
            static_cast<ani_object>(property), "longValue", longValueSig.c_str(), &value);
        if (status != ANI_OK) {
            return { false, nullopt };
        }
        auto result = make_optional<int64_t>(static_cast<int64_t>(value));
        return { true, move(result) };
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
                HILOGE("vm GetEnv, err: %{private}d", status);
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
                HILOGE("Detach thread env from vm failed! status: %{private}d", status);
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

        auto& mainHandler = GetMainHandler();
        if (mainHandler == nullptr) {
            shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
            if (!runner) {
                HILOGE("get main event runner failed!");
                return false;
            }
            mainHandler = CreateSharedPtr<OHOS::AppExecFwk::EventHandler>(runner);
            if (mainHandler == nullptr) {
                HILOGE("Failed to request heap memory.");
                return false;
            }
        }
        bool succ = mainHandler->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::HIGH, {});
        if (!succ) {
            HILOGE("Failed to post task to main thread.");
            return false;
        }
        return true;
    }
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI
#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_HELPER_H