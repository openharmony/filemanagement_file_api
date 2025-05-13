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

#include "movedir_ani.h"

#include <optional>
#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "movedir_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static tuple<bool, ani_object> ToConflictFiles(ani_env *env, const ErrFiles &files)
{
    auto classDesc = FS::ConflictFilesInner::classDesc.c_str();
    ani_class cls;
    ani_status ret;
    if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
        HILOGE("Cannot find class %{private}s, err: %{private}d", classDesc, ret);
        return { false, nullptr };
    }

    auto ctorDesc = FS::ConflictFilesInner::ctorDesc.c_str();
    auto ctorSig = FS::ConflictFilesInner::ctorSig.c_str();
    ani_method ctor;
    if ((ret = env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) != ANI_OK) {
        HILOGE("Cannot find class %{private}s constructor method, err: %{private}d", classDesc, ret);
        return { false, nullptr };
    }

    auto [succSrc, src] = TypeConverter::ToAniString(env, files.srcFiles);
    if (!succSrc) {
        HILOGE("Convert ConflictFiles srcFiles to ani string failed!");
        return { false, nullptr };
    }

    auto [succDest, dest] = TypeConverter::ToAniString(env, files.destFiles);
    if (!succSrc) {
        HILOGE("Convert ConflictFiles destFiles to ani string failed!");
        return { false, nullptr };
    }

    ani_object obj;
    if ((ret = env->Object_New(cls, ctor, &obj, src, dest)) != ANI_OK) {
        HILOGE("Create ConflictFiles object failed!, err: %{private}d", ret);
        return { false, nullptr };
    }

    return { true, obj };
}

static tuple<bool, optional<ani_object>> ToConflictFilesArray(
    ani_env *env, const optional<deque<struct ErrFiles>> &errFiles)
{
    if (!errFiles.has_value()) {
        return { true, nullopt };
    }
    auto classDesc = BuiltInTypes::Array::classDesc.c_str();
    ani_class cls = nullptr;
    ani_status ret;

    if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
        HILOGE("Cannot find class %{private}s, err: %{private}d", classDesc, ret);
        return { false, nullopt };
    }

    auto ctorDesc = BuiltInTypes::Array::ctorDesc.c_str();
    auto ctorSig = BuiltInTypes::Array::ctorSig.c_str();
    ani_method ctor;
    if ((ret = env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) != ANI_OK) {
        HILOGE("Cannot find class %{private}s constructor method, err: %{private}d", classDesc, ret);
        return { false, nullopt };
    }

    ani_object arr;
    auto files = errFiles.value();
    if ((ret = env->Object_New(cls, ctor, &arr, files.size())) != ANI_OK) {
        HILOGE("Create Array failed!, err: %{private}d", ret);
        return { false, nullopt };
    }

    auto setterDesc = BuiltInTypes::Array::setterDesc.c_str();
    auto setterSig = BuiltInTypes::Array::objectSetterSig.c_str();
    ani_size index = 0;
    for (const auto &errFile : files) {
        auto [succ, fileObj] = ToConflictFiles(env, errFile);
        if (!succ) {
            return { false, nullopt };
        }

        if ((ret = env->Object_CallMethodByName_Void(arr, setterDesc, setterSig, index, fileObj)) != ANI_OK) {
            HILOGE("Add element to Array failed, err: %{private}d", ret);
            return { false, nullopt };
        }
        index++;
    }

    return { true, make_optional<ani_object>(move(arr)) };
}

void MoveDirAni::MoveDirSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string src, ani_string dest, ani_object mode)
{
    auto [succSrc, srcPath] = TypeConverter::ToUTF8String(env, src);
    auto [succDest, destPath] = TypeConverter::ToUTF8String(env, dest);
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [succMode, optMode] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Failed to convert mode to int32");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [fsResult, errFiles] = MoveDirCore::DoMoveDir(srcPath, destPath, optMode);
    if (!fsResult.IsSuccess()) {
        HILOGE("DoMoveDir failed!");
        auto [succ, errData] = ToConflictFilesArray(env, errFiles);
        if (!succ) {
            HILOGE("Convert conflict files array failed");
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return;
        }
        const FsError &err = fsResult.GetError();
        ErrorHandler::Throw(env, err, errData);
        return;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS