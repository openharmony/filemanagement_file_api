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

#include "stat_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "stat_core.h"
#include "stat_wrapper.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

ani_object StatAni::StatSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_object file)
{
    auto [succPath, fileInfo] = TypeConverter::ToFileInfo(env, file);
    if (!succPath) {
        HILOGE("The first argument requires filepath/fd");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto ret = StatCore::DoStat(fileInfo);
    if (!ret.IsSuccess()) {
        HILOGE("DoStat failed!");
        const FsError &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto fsStat = ret.GetData().value();
    auto statObject = StatWrapper::Wrap(env, fsStat);
    if (statObject == nullptr) {
        delete fsStat;
        fsStat = nullptr;
        HILOGE("Wrap stat object failed!");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return statObject;
}

ani_boolean StatAni::IsBlockDevice(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsBlockDevice();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsCharacterDevice(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsCharacterDevice();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsDirectory(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsDirectory();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsFIFO(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsFIFO();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsFile(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsFile();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsSocket(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsSocket();
    return ani_boolean(ret);
}

ani_boolean StatAni::IsSymbolicLink(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return ANI_FALSE;
    }

    auto ret = fsStat->IsSymbolicLink();
    return ani_boolean(ret);
}

ani_object StatAni::GetIno(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    auto ino = fsStat->GetIno();

    // AniCache& aniCache = AniCache::GetInstance();
    // auto [succ, cls] = aniCache.GetClass(env, BuiltInTypes::BigInt::classDesc);
    // if (succ != ANI_OK) {
    //     return nullptr;
    // }

    // ani_method ctor;
    // tie(succ, ctor) = aniCache.GetMethod(env, BuiltInTypes::BigInt::classDesc, BuiltInTypes::BigInt::ctorDesc, BuiltInTypes::BigInt::ctorSig);
    // if (succ != ANI_OK) {
    //     return nullptr;
    // }

    // ani_object inoObject;

    // if ((succ = env->Object_New(cls, ctor, &inoObject, ino)) != ANI_OK) {
    //     HILOGE("New Object Fail, err: %{public}d", succ);
    //     return nullptr;
    // }

    auto [ret, inoObject] = TypeConverter::ToAniBigInt(env, ino);
    return inoObject;
}

ani_long StatAni::GetMode(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    HILOGE("GetMode!!!");

    auto mode = fsStat->GetMode();
    return mode;
}

ani_long StatAni::GetUid(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsStat = StatWrapper::Unwrap(env, object);
    if (fsStat == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return -1;
    }

    auto uid = fsStat->GetUid();
    return uid;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS