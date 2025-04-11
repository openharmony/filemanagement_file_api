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

#ifndef FILEMANAGEMENT_ANI_ERROR_HANDLER_H
#define FILEMANAGEMENT_ANI_ERROR_HANDLER_H

#include <cstdint>
#include <string>
#include <ani.h>
#include "ani_helper.h"
#include "filemgmt_libhilog.h"
#include "fs_error.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {

class ErrorHandler {
public:
    static ani_status Throw(
        ani_env *env, int32_t code, const std::string &errMsg, const std::optional<ani_object> &errData = std::nullopt);

    static ani_status Throw(ani_env *env, int32_t code, const std::optional<ani_object> &errData = std::nullopt)
    {
        if (env == nullptr) {
            HILOGE("Invalid parameter env");
            return ANI_INVALID_ARGS;
        }
        FsError err(code);
        return Throw(env, std::move(err), errData);
    }

    static ani_status Throw(ani_env *env, const FsError &err, const std::optional<ani_object> &errData = std::nullopt)
    {
        if (env == nullptr) {
            HILOGE("Invalid parameter env");
            return ANI_INVALID_ARGS;
        }
        auto code = err.GetErrNo();
        const auto &errMsg = err.GetErrMsg();
        return Throw(env, code, errMsg, errData);
    }

private:
    static ani_status Throw(ani_env *env, const char *className, int32_t code, const std::string &errMsg,
        const std::optional<ani_object> &errData = std::nullopt)
    {
        if (env == nullptr) {
            HILOGE("Invalid parameter env");
            return ANI_INVALID_ARGS;
        }

        if (className == nullptr) {
            HILOGE("Invalid parameter className");
            return ANI_INVALID_ARGS;
        }

        auto [status, err] = CreateErrorObj(env, className, code, errMsg, errData);

        if (status != ANI_OK) {
            HILOGE("Create error object failed!");
            return status;
        }

        status = env->ThrowError(err);
        if (status != ANI_OK) {
            HILOGE("Throw ani error object failed!");
            return status;
        }
        return ANI_OK;
    }

    static std::tuple<ani_status, ani_error> CreateErrorObj(ani_env *env, const char *className, int32_t code,
        const std::string &errMsg, const std::optional<ani_object> &errData = std::nullopt)
    {
        ani_class cls;
        if (ANI_OK != env->FindClass(className, &cls)) {
            HILOGE("Cannot find class '%{private}s'", className);
            return { ANI_NOT_FOUND, nullptr };
        }

        ani_method ctor;
        if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", ":V", &ctor)) {
            HILOGE("Cannot find constructor for class '%{private}s'", className);
            return { ANI_NOT_FOUND, nullptr };
        }

        ani_object obj;
        if (ANI_OK != env->Object_New(cls, ctor, &obj)) {
            HILOGE("Cannot create ani error object");
            return { ANI_ERROR, nullptr };
        }

        auto [succ, message] = TypeConverter::ToAniString(env, errMsg);
        if (!succ) {
            HILOGE("Convert errMsg to ani string failed");
            return { ANI_ERROR, nullptr };
        }

        ani_status status = ANI_ERROR;

        status = env->Object_SetPropertyByName_Ref(obj, "message", message);
        if (status != ANI_OK) {
            HILOGE("Set property 'message' value failed");
            return { status, nullptr };
        }

        status = env->Object_SetPropertyByName_Double(obj, "code", static_cast<double>(code));
        if (status != ANI_OK) {
            HILOGE("Set property 'code' value failed");
            return { status, nullptr };
        }

        if (errData.has_value()) {
            status = env->Object_SetPropertyByName_Ref(obj, "data", errData.value());
            if (status != ANI_OK) {
                HILOGE("Set property 'data' value failed");
                return { status, nullptr };
            }
        }

        ani_error err = static_cast<ani_error>(obj);
        return { ANI_OK, std::move(err) };
    }
};

} // namespace OHOS::FileManagement::ModuleFileIO::ANI

#endif // FILEMANAGEMENT_ANI_ERROR_HANDLER_H