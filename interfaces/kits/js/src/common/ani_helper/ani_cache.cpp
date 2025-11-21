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

#include "ani_cache.h"

#include <tuple>

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;

AniCache& AniCache::GetInstance()
{
    static AniCache instance;
    return instance;
}

tuple<ani_status, ani_class> AniCache::GetClass(ani_env *env, const string &name)
{
    lock_guard<mutex> lock(exClassMapLock);
    if (clazzMap.find(name) != clazzMap.end()) {
        return {ANI_OK, static_cast<ani_class>(clazzMap[name])};
    }

    if (env == nullptr) {
        HILOGE("env is nullptr");
        return {ANI_ERROR, nullptr};
    }
    ani_class clsCalc{};
    const char *className = name.c_str();
    ani_status ret;
    if (ANI_OK != (ret = env->FindClass(className, &clsCalc))) {
        HILOGE("Not found %{public}s, err:%{public}d", className, ret);
        return { ret, nullptr };
    }
    ani_ref gCalc{};
    if (ANI_OK != (ret = env->GlobalReference_Create(static_cast<ani_ref>(clsCalc), &gCalc))) {
        HILOGE("Failed to GlobalRefrence_Create: %{public}s, err: %{public}d", className, ret);
        return { ret, nullptr };
    }
    clazzMap[name] = gCalc;

    return { ret, static_cast<ani_class>(gCalc)};
}

tuple<ani_status, ani_enum> AniCache::GetEnum(ani_env *env, const string &name)
{
    lock_guard<mutex> lock(exClassMapLock);
    if (clazzMap.find(name) != clazzMap.end()) {
        return { ANI_OK, static_cast<ani_enum>(clazzMap[name]) };
    }

    if (env == nullptr) {
        HILOGE("env is nullptr");
        return {ANI_ERROR, nullptr};
    }
    ani_enum clsCalc{};
    const char *className = name.c_str();
    ani_status ret;
    if (ANI_OK != (ret = env->FindEnum(className, &clsCalc))) {
        HILOGE("Not found %{public}s, err:%{public}d", className, ret);
        return { ret, nullptr };
    }
    ani_ref gCalc{};
    if (ANI_OK != (ret = env->GlobalReference_Create(static_cast<ani_ref>(clsCalc), &gCalc))) {
        HILOGE("Failed to GlobalReference_Create: %{public}s, err: %{public}d", className, ret);
        return { ret, nullptr };
    }

    clazzMap[name] = gCalc;

    return { ret, static_cast<ani_enum>(gCalc) };
}
tuple<ani_status, ani_method> AniCache::GetMethod(ani_env *env, const string &clazzName, const string &methodName,
    const string& methodSignature)
{
    auto [ret, cls] = GetClass(env, clazzName);
    if (ret != ANI_OK) {
        return { ret, nullptr };
    }
    ani_method ctor;
    if (ANI_OK != (ret = env->Class_FindMethod(cls, methodName.c_str(), methodSignature.c_str(), &ctor))) {
        HILOGE("Not found ctor, methodName: %{public}s, signature: %{public}s, err: %{public}d",
            methodName.c_str(),
            methodSignature.c_str(),
            ret);
        return { ret, nullptr };
    }
    return { ANI_OK, ctor };
}

tuple<ani_status, ani_enum_item> AniCache::GetEnumIndex(ani_env *env, const string &enumName, int index)
{
    auto [ret, enumType] = GetEnum(env, enumName);
    if (ret != ANI_OK) {
        return { ret, nullptr };
    }

    ani_enum_item enumItem;
    ret = env->Enum_GetEnumItemByIndex(enumType, index, &enumItem);
    if (ret != ANI_OK) {
        HILOGE("Enum_GetEnumItemByIndex failed, index: %{public}d, err: %{public}d", index, ret);
        return { ret, nullptr };
    }
    return { ANI_OK, enumItem };
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI