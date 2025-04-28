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

#include "create_randomaccessfile_ani.h"

#include "ani_helper.h"
#include "create_randomaccessfile_core.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

static ani_object Wrap(ani_env *env, const FsRandomAccessFile *rafFile)
{
    static const char *className = "L@ohos/file/fs/fileIo/RandomAccessFileInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        HILOGE("Cannot find class %s", className);
        return nullptr;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "J:V", &ctor)) {
        HILOGE("Cannot find constructor method for class %s", className);
        return nullptr;
    }
    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(rafFile));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", className);
        return nullptr;
    }
    const auto &fdRet = rafFile->GetFD();
    if (!fdRet.IsSuccess()) {
        HILOGE("GetFD Failed!");
        return nullptr;
    }
    const auto &fd = fdRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "fd", static_cast<double>(fd))) {
        HILOGE("Set fd field value failed!");
        return nullptr;
    }
    const auto &fpRet = rafFile->GetFPointer();
    if (!fpRet.IsSuccess()) {
        HILOGE("GetFPointer Failed!");
        return nullptr;
    }
    const auto &fp = fpRet.GetData().value();
    if (ANI_OK != AniHelper::SetPropertyValue(env, cls, obj, "filePointer", static_cast<double>(fp))) {
        HILOGE("Set fp field value failed!");
        return nullptr;
    }
    return obj;
}

static tuple<bool, bool> JudgeFile(ani_env *env, ani_object obj)
{
    ani_class stringClass;
    env->FindClass("Lstd/core/String;", &stringClass);
    ani_boolean isString = false;
    env->Object_InstanceOf(obj, stringClass, &isString);
    if (isString) {
        return { true, true };
    }

    ani_class fileClass;
    env->FindClass("L@ohos/file/fs/fileIo/FileInner;", &fileClass);
    ani_boolean isFile = false;
    env->Object_InstanceOf(obj, fileClass, &isFile);
    if (isFile) {
        return { true, false };
    }
    HILOGE("Invalid file type");
    return { false, false };
}

static tuple<bool, optional<RandomAccessFileOptions>> ToRafOptions(ani_env *env, ani_object obj)
{
    RandomAccessFileOptions options;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succStart, start] = AniHelper::ParseInt64Option(env, obj, "start");
    if (!succStart) {
        HILOGE("Illegal option.start parameter");
        return { false, nullopt };
    }
    options.start = start;

    auto [succEnd, end] = AniHelper::ParseInt64Option(env, obj, "end");
    if (!succEnd) {
        HILOGE("Illegal option.end parameter");
        return { false, nullopt };
    }
    options.end = end;

    return { true, make_optional<RandomAccessFileOptions>(move(options)) };
}

static ani_object CreateRandomAccessFileByString(ani_env *env, ani_object file, ani_object mode,
    optional<RandomAccessFileOptions> op)
{
    auto [succPath, path] = TypeConverter::ToUTF8String(env, static_cast<ani_string>(file));
    if (!succPath) {
        HILOGE("Parse file path failed");
        return nullptr;
    }
    auto [succMode, modeOp] = TypeConverter::ToOptionalInt32(env, mode);
    if (!succMode) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    FsResult<FsRandomAccessFile *> ret = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, modeOp, op);
    if (!ret.IsSuccess()) {
        HILOGE("CreateRandomAccessFile failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    const FsRandomAccessFile *refFile = ret.GetData().value();
    auto result = Wrap(env, move(refFile));
    if (result == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

ani_object CreateRandomAccessFileAni::CreateRandomAccessFileSync(ani_env *env, [[maybe_unused]] ani_class clazz,
                                                                 ani_object file, ani_object mode, ani_object options)
{
    auto [succOp, op] = ToRafOptions(env, options);
    if (!succOp) {
        HILOGE("Failed to resolve options!");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [succ, isPath] = JudgeFile(env, file);
    if (!succ) {
        HILOGE("Judge file argument failed");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    if (isPath) {
        return CreateRandomAccessFileByString(env, file, mode, op);
    } else {
        ani_double fdOp;
        if (ANI_OK != env->Object_GetPropertyByName_Double(file, "fd", &fdOp)) {
            HILOGE("Get fd in class file failed");
            ErrorHandler::Throw(env, EINVAL);
            return nullptr;
        }
        int32_t fd = static_cast<int32_t>(fdOp);
        FsResult<FsRandomAccessFile *> ret = CreateRandomAccessFileCore::DoCreateRandomAccessFile(fd, op);
        if (!ret.IsSuccess()) {
            HILOGE("CreateRandomAccessFile failed");
            const auto &err = ret.GetError();
            ErrorHandler::Throw(env, err);
            return nullptr;
        }
        const FsRandomAccessFile *refFile = ret.GetData().value();
        auto result = Wrap(env, move(refFile));
        if (result == nullptr) {
            ErrorHandler::Throw(env, UNKNOWN_ERR);
            return nullptr;
        }
        return result;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
