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

static ani_status SetNumProperty(
    ani_env *env, const ani_class &cls, ani_object &object, const char *name, ani_double &value)
{
    ani_method setter;
    ani_status ret;
    if ((ret = env->Class_FindMethod(cls, name, nullptr, &setter)) != ANI_OK) {
        HILOGE("Class_FindMethod Fail %{private}s, err: %{private}d", name, ret);
        return ret;
    }

    return env->Object_CallMethod_Void(object, setter, value);
}

static ani_status SetBigIntProperty(
    ani_env *env, const ani_class &statCls, ani_object &statObject, const char *name, ani_double &value)
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

    ani_method setter;
    if ((ret = env->Class_FindMethod(statCls, name, nullptr, &setter)) != ANI_OK) {
        HILOGE("Class_FindMethod Fail %{private}s, err: %{private}d", name, ret);
        return ret;
    }

    return env->Object_CallMethod_Void(statObject, setter, object);
}

static ani_int GetLocationEnumIndex(const Location &value)
{
    switch (value) {
        case LOCAL:
            return ani_int(0);
        case CLOUD:
            return ani_int(1);
    }
}

static ani_status SetEnumLocation(
    ani_env *env, const ani_class &cls, ani_object &object, const char *name, const Location &value)
{
    ani_method setter;
    ani_status ret;
    if ((ret = env->Class_FindMethod(cls, name, nullptr, &setter)) != ANI_OK) {
        HILOGE("Class_FindMethod Fail %{private}s, err: %{private}d", name, ret);
        return ret;
    }

    return env->Object_CallMethod_Void(object, setter, GetLocationEnumIndex(value));
}

const static string MODE_SETTER = Builder::BuildSetterName("mode");
const static string UID_SETTER = Builder::BuildSetterName("uid");
const static string GID_SETTER = Builder::BuildSetterName("gid");
const static string SIZE_SETTER = Builder::BuildSetterName("size");
const static string ATIME_SETTER = Builder::BuildSetterName("atime");
const static string MTIME_SETTER = Builder::BuildSetterName("mtime");
const static string CTIME_SETTER = Builder::BuildSetterName("ctime");
const static string INO_SETTER = Builder::BuildSetterName("ino");
const static string ATIME_NS_SETTER = Builder::BuildSetterName("atimeNs");
const static string MTIME_NS_SETTER = Builder::BuildSetterName("mtimeNs");
const static string CTIME_NS_SETTER = Builder::BuildSetterName("ctimeNs");
const static string LOCATION_SETTER = Builder::BuildSetterName("location");

static ani_status SetProperties(ani_env *env, const ani_class &cls, ani_object &statObject, FsStat *fsStat)
{
    ani_status ret;

    vector<pair<string_view, ani_double>> numProperties = {
        { MODE_SETTER, ani_double(static_cast<double>(fsStat->GetMode())) },
        { UID_SETTER, ani_double(static_cast<double>(fsStat->GetUid())) },
        { GID_SETTER, ani_double(static_cast<double>(fsStat->GetGid())) },
        { SIZE_SETTER, ani_double(static_cast<double>(fsStat->GetSize())) },
        { ATIME_SETTER, ani_double(static_cast<double>(fsStat->GetAtime())) },
        { MTIME_SETTER, ani_double(static_cast<double>(fsStat->GetMtime())) },
        { CTIME_SETTER, ani_double(static_cast<double>(fsStat->GetCtime())) },
    };
    for (auto iter : numProperties) {
        auto key = iter.first.data();
        auto value = iter.second;
        ret = SetNumProperty(env, cls, statObject, key, value);
        if (ret != ANI_OK) {
            HILOGE("Object_CallMethod_Void Fail %{private}s, err: %{private}d", key, ret);
            return ret;
        }
    }

    vector<pair<string_view, ani_double>> bigIntProperties = {
        { INO_SETTER, ani_double(static_cast<double>(fsStat->GetIno())) },
        { ATIME_NS_SETTER, ani_double(static_cast<double>(fsStat->GetAtimeNs())) },
        { MTIME_NS_SETTER, ani_double(static_cast<double>(fsStat->GetMtimeNs())) },
        { CTIME_NS_SETTER, ani_double(static_cast<double>(fsStat->GetCtimeNs())) },
    };
    for (auto iter : bigIntProperties) {
        auto key = iter.first.data();
        auto value = iter.second;
        ret = SetBigIntProperty(env, cls, statObject, key, value);
        if (ret != ANI_OK) {
            HILOGE("Object_CallMethod_Void Fail %{private}s, err: %{private}d", key, ret);
            return ret;
        }
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    auto key = LOCATION_SETTER.c_str();
    if ((ret = SetEnumLocation(env, cls, statObject, key, static_cast<Location>(fsStat->GetLocation()))) != ANI_OK) {
        HILOGE("Object_CallMethod_Void Fail %{private}s, err: %{private}d", key, ret);
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

    if ((ret = SetProperties(env, cls, statObject, fsStat)) != ANI_OK) {
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