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

#include "stat_wrapper.h"

#include <iostream>
#include <string>
#include <string_view>

#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static ani_status SetBigIntProperty(ani_env *env, ani_object &statObject, const char *name, int64_t value)
{
    ani_object object = {};
    auto classDesc = BuiltInTypes::BigInt::classDesc.c_str();
    ani_class cls;
    ani_status ret;

    if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
        HILOGE("Not found %{private}s, err: %{private}d", classDesc, ret);
        return ret;
    }

    auto ctorDesc = BuiltInTypes::BigInt::ctorDesc.c_str();
    auto ctorSig = BuiltInTypes::BigInt::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Not found ctor, err: %{private}d", ret);
        return ret;
    }

    if ((ret = env->Object_New(cls, ctor, &object, value)) != ANI_OK) {
        HILOGE("New BigIntProperty Fail, err: %{private}d", ret);
        return ret;
    }

    ret = AniHelper::SetPropertyValue(env, statObject, name, object);
    if (ret != ANI_OK) {
        HILOGE("SetPropertyValue Fail %{private}s, err: %{private}d", name, ret);
        return ret;
    }

    return ANI_OK;
}

static ani_enum_item GetLocationEnumIndex(ani_env *env, const Location &value)
{
    ani_enum enumType;
    auto classDesc = FS::LocationType::classDesc.c_str();
    ani_status ret = env->FindEnum(classDesc, &enumType);
    if (ret != ANI_OK) {
        HILOGE("FindEnum %{private}s failed, err: %{private}d", classDesc, ret);
        return nullptr;
    }

    size_t valueAsSizeT = static_cast<size_t>(value);
    if (valueAsSizeT < 1) {
        HILOGE("Invalid Location value: %{private}zu", valueAsSizeT);
        return nullptr;
    }

    size_t index = valueAsSizeT - 1;

    ani_enum_item enumItem;
    ret = env->Enum_GetEnumItemByIndex(enumType, index, &enumItem);
    if (ret != ANI_OK) {
        HILOGE("Enum_GetEnumItemByIndex failed, index: %{private}zu, err: %{private}d", index, ret);
        return nullptr;
    }
    return enumItem;
}

static ani_status SetEnumLocation(ani_env *env, ani_object &object, const char *name, const Location &value)
{
    ani_enum_item location = GetLocationEnumIndex(env, value);
    if (location == nullptr) {
        return ANI_ERROR;
    }

    ani_status ret = AniHelper::SetPropertyValue(env, object, name, location);
    if (ret != ANI_OK) {
        HILOGE("SetPropertyValue Fail %{private}s, err: %{private}d", name, ret);
        return ret;
    }

    return ANI_OK;
}

static ani_status SetProperties(ani_env *env, ani_object &statObject, FsStat *fsStat)
{
    ani_status ret;

    vector<pair<string_view, int64_t>> numProperties = {
        { "mode", fsStat->GetMode() },
        { "uid", fsStat->GetUid() },
        { "gid", fsStat->GetGid() },
        { "size", fsStat->GetSize() },
        { "atime", fsStat->GetAtime() },
        { "mtime", fsStat->GetMtime() },
        { "ctime", fsStat->GetCtime() },
    };
    for (auto iter : numProperties) {
        auto key = iter.first.data();
        auto value = iter.second;
        ret = AniHelper::SetPropertyValue(env, statObject, key, value);
        if (ret != ANI_OK) {
            HILOGE("SetPropertyValue Fail %{private}s, err: %{private}d", key, ret);
            return ret;
        }
    }

    vector<pair<string_view, int64_t>> bigIntProperties = {
        { "ino", fsStat->GetIno() },
        { "atimeNs", fsStat->GetAtimeNs() },
        { "mtimeNs", fsStat->GetMtimeNs() },
        { "ctimeNs", fsStat->GetCtimeNs() },
    };
    for (auto iter : bigIntProperties) {
        auto key = iter.first.data();
        auto value = iter.second;
        ret = SetBigIntProperty(env, statObject, key, value);
        if (ret != ANI_OK) {
            HILOGE("SetBigIntProperty Fail %{private}s, err: %{private}d", key, ret);
            return ret;
        }
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if ((ret = SetEnumLocation(env, statObject, "location", static_cast<Location>(fsStat->GetLocation()))) != ANI_OK) {
        HILOGE("SetEnumLocation Fail, err: %{private}d", ret);
        return ret;
    }
#endif

    return ANI_OK;
}

ani_object StatWrapper::Wrap(ani_env *env, FsStat *fsStat)
{
    if (fsStat == nullptr) {
        HILOGE("FsStat pointer is null!");
        return nullptr;
    }
    auto classDesc = FS::StatInner::classDesc.c_str();
    ani_object statObject = {};
    ani_class cls;
    ani_status ret;

    if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
        HILOGE("Not found %{private}s, err: %{private}d", classDesc, ret);
        return nullptr;
    }

    auto ctorDesc = FS::StatInner::ctorDesc.c_str();
    auto ctorSig = FS::StatInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Not found ctor, err: %{private}d", ret);
        return nullptr;
    }

    if ((ret = env->Object_New(cls, ctor, &statObject, reinterpret_cast<ani_long>(fsStat))) != ANI_OK) {
        HILOGE("New StatInner Fail, err: %{private}d", ret);
        return nullptr;
    }

    if ((ret = SetProperties(env, statObject, fsStat)) != ANI_OK) {
        HILOGE("SetProperties Fail, err: %{private}d", ret);
        return nullptr;
    }

    return statObject;
}

FsStat *StatWrapper::Unwrap(ani_env *env, ani_object object)
{
    ani_long fsStat;
    auto ret = env->Object_GetFieldByName_Long(object, "nativeStat", &fsStat);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsStat err: %{private}d", ret);
        return nullptr;
    }
    return reinterpret_cast<FsStat *>(fsStat);
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS