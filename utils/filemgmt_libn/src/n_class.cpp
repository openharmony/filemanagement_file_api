/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "n_class.h"

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;
NClass &NClass::GetInstance()
{
    static thread_local NClass nClass;
    return nClass;
}

tuple<bool, napi_value> NClass::DefineClass(napi_env env,
                                            string className,
                                            napi_callback constructor,
                                            vector<napi_property_descriptor> &&properties)
{
    napi_value classVal = nullptr;
    napi_status stat = napi_define_class(env, className.c_str(), className.length(), constructor, nullptr,
                                         properties.size(), properties.data(), &classVal);
    if (stat != napi_ok) {
        HILOGE("INNER BUG. Cannot define class %{public}s because of %{public}d", className.c_str(), stat);
    }

    return {stat == napi_ok, classVal};
}

bool NClass::SaveClass(napi_env env, string className, napi_value exClass)
{
    NClass &nClass = NClass::GetInstance();
    lock_guard<std::mutex>(nClass.exClassMapLock);

    if (nClass.exClassMap.find(className) != nClass.exClassMap.end()) {
        return true;
    }

    napi_ref constructor;
    napi_status res = napi_create_reference(env, exClass, 1, &constructor);
    if (res == napi_ok) {
        nClass.exClassMap.insert({className, constructor});
    } else {
        HILOGE("INNER BUG. Cannot ref class constructor %{public}s because of %{public}d", className.c_str(), res);
    }

    if (!nClass.addCleanHook) {
        napi_status status = napi_add_env_cleanup_hook(env, CleanClass, env);
        if (status != napi_ok) {
            HILOGE("INNER BUG. Cleanup_hook registation has failed because of %{public}d", res);
        } else {
            nClass.addCleanHook = true;
        }
    }
    return res == napi_ok;
}

void NClass::CleanClass(void *arg)
{
    napi_env env = reinterpret_cast<napi_env>(arg);
    NClass &nClass = NClass::GetInstance();
    lock_guard<std::mutex>(nClass.exClassMapLock);
    {
        lock_guard<std::mutex>(nClass.wrapLock);
        nClass.wrapReleased = true;
    }
    napi_status res;
    for (auto it = nClass.exClassMap.begin(); it != nClass.exClassMap.end(); ++it) {
        res = napi_delete_reference(env, it->second);
        if (res != napi_ok) {
            HILOGE("Cannot del ref class constructor %{public}s because of %{public}d", it->first.c_str(), res);
        }
    }
    nClass.exClassMap.clear();
}

napi_value NClass::InstantiateClass(napi_env env, const string& className, const vector<napi_value>& args)
{
    NClass &nClass = NClass::GetInstance();
    lock_guard<std::mutex>(nClass.exClassMapLock);

    auto it = nClass.exClassMap.find(className);
    if (it == nClass.exClassMap.end()) {
        HILOGE("Class %{public}s hasn't been saved yet", className.c_str());
        return nullptr;
    }

    napi_value cons = nullptr;
    napi_status status = napi_get_reference_value(env, it->second, &cons);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot deref class %{public}s because of %{public}d", className.c_str(), status);
        return nullptr;
    }

    napi_value instance = nullptr;
    status = napi_new_instance(env, cons, args.size(), args.data(), &instance);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot instantiate the class %{public}s because of %{public}d", className.c_str(), status);
        return nullptr;
    }

    return instance;
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
