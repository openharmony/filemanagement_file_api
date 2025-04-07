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

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

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
    static const char *className = "Lescompat/BigInt;";
    ani_class cls;
    ani_status ret;

    if ((ret = env->FindClass(className, &cls)) != ANI_OK) {
        HILOGE("Not found %{private}s, err: %{private}d", className, ret);
        return ret;
    }

    ani_method ctor;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", "D:V", &ctor)) != ANI_OK) {
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

static ani_status SetProperties(ani_env *env, const ani_class &cls, ani_object &statObject, FsStat *fsStat)
{
    ani_status ret;

    vector<pair<const char *, ani_double>> numProperties = {
        { "<set>mode", ani_double(static_cast<double>(fsStat->GetMode())) },
        { "<set>uid", ani_double(static_cast<double>(fsStat->GetUid())) },
        { "<set>gid", ani_double(static_cast<double>(fsStat->GetGid())) },
        { "<set>size", ani_double(static_cast<double>(fsStat->GetSize())) },
        { "<set>atime", ani_double(static_cast<double>(fsStat->GetAtime())) },
        { "<set>mtime", ani_double(static_cast<double>(fsStat->GetMtime())) },
        { "<set>ctime", ani_double(static_cast<double>(fsStat->GetCtime())) },
    };
    for (auto iter : numProperties) {
        ret = SetNumProperty(env, cls, statObject, iter.first, iter.second);
        if (ret != ANI_OK) {
            HILOGE("Object_CallMethod_Void Fail %{private}s, err: %{private}d", iter.first, ret);
            return ret;
        }
    }

    vector<pair<const char *, ani_double>> bigIntProperties = {
        { "<set>ino", ani_double(static_cast<double>(fsStat->GetIno())) },
        { "<set>atimeNs", ani_double(static_cast<double>(fsStat->GetAtimeNs())) },
        { "<set>mtimeNs", ani_double(static_cast<double>(fsStat->GetMtimeNs())) },
        { "<set>ctimeNs", ani_double(static_cast<double>(fsStat->GetCtimeNs())) },
    };
    for (auto iter : bigIntProperties) {
        ret = SetBigIntProperty(env, cls, statObject, iter.first, iter.second);
        if (ret != ANI_OK) {
            HILOGE("Object_CallMethod_Void Fail %{private}s, err: %{private}d", iter.first, ret);
            return ret;
        }
    }

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if ((ret = SetEnumLocation(env, cls, statObject, "<set>location", static_cast<Location>(fsStat->GetLocation()))) !=
        ANI_OK) {
        HILOGE("Object_CallMethod_Void Fail <set>location, err: %{private}d", ret);
        return ret;
    }
#endif

    return ANI_OK;
}

tuple<ani_status, ani_object> StatWrapper::Wrap(ani_env *env, FsStat *fsStat)
{
    ani_object statObject = {};
    static const char *className = "L@ohos/file/fs/fileIo/StatInner;";
    ani_class cls;
    ani_status ret;

    if ((ret = env->FindClass(className, &cls)) != ANI_OK) {
        HILOGE("Not found %{private}s, err: %{private}d", className, ret);
        return { ret, statObject };
    }

    ani_method ctor;
    if ((ret = env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) != ANI_OK) {
        HILOGE("Not found ctor, err: %{private}d", ret);
        return { ret, statObject };
    }

    if ((ret = env->Object_New(cls, ctor, &statObject, reinterpret_cast<ani_long>(fsStat))) != ANI_OK) {
        HILOGE("New StatInner Fail, err: %{private}d", ret);
        return { ret, statObject };
    }

    if ((ret = SetProperties(env, cls, statObject, fsStat)) != ANI_OK) {
        HILOGE("SetProperties Fail, err: %{private}d", ret);
        return { ret, statObject };
    }

    return { ANI_OK, statObject };
}

FsStat* StatWrapper::Unwrap(ani_env *env, ani_object object)
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