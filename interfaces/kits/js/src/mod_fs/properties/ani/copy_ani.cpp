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

#include "copy_ani.h"

#include "ani_helper.h"
#include "ani_signature.h"
#include "copy_core.h"
#include "error_handler.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_task_signal.h"
#include "progress_listener_ani.h"
#include "task_signal_listener_ani.h"
#include "task_signal_wrapper.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static bool ParseListenerFromOptionArg(ani_env *env, const ani_object &options, CopyOptions &opts)
{
    ani_ref prog;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, "progressListener", &prog)) {
        HILOGE("Illegal options.progressListener type");
        return false;
    }

    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(prog, &isUndefined);
    if (isUndefined) {
        return true;
    }

    ani_ref cbRef;
    if (ANI_OK != env->GlobalReference_Create(prog, &cbRef)) {
        HILOGE("Failed to create reference");
        return false;
    }

    ani_vm *vm = nullptr;
    env->GetVM(&vm);
    auto listener = CreateSharedPtr<ProgressListenerAni>(vm, cbRef);
    if (listener == nullptr) {
        HILOGE("Failed to request heap memory.");
        return false;
    }

    opts.progressListener = move(listener);
    return true;
}

static bool ParseCopySignalFromOptionArg(ani_env *env, const ani_object &options, CopyOptions &opts)
{
    ani_ref prog;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(options, "copySignal", &prog)) {
        HILOGE("Illegal options.CopySignal type");
        return false;
    }

    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(prog, &isUndefined);
    if (isUndefined) {
        return true;
    }

    FsTaskSignal *copySignal = TaskSignalWrapper::Unwrap(env, static_cast<ani_object>(prog));
    if (copySignal != nullptr) {
        opts.copySignal = copySignal;
    }

    return true;
}

static tuple<bool, optional<CopyOptions>> ParseOptions(ani_env *env, ani_object &options)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(options, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    CopyOptions opts;
    auto succ = ParseListenerFromOptionArg(env, options, opts);
    if (!succ) {
        return { false, nullopt };
    }

    succ = ParseCopySignalFromOptionArg(env, options, opts);
    if (!succ) {
        return { false, nullopt };
    }

    return { true, make_optional(move(opts)) };
}

void CopyAni::CopySync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string srcUri, ani_string destUri, ani_object options)
{
    auto [succSrc, src] = TypeConverter::ToUTF8String(env, srcUri);
    auto [succDest, dest] = TypeConverter::ToUTF8String(env, destUri);
    auto [succOpts, opts] = ParseOptions(env, options);

    if (!succSrc) {
        HILOGE("The first argument requires uri");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
    if (!succDest) {
        HILOGE("The second argument requires uri");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }
    if (!succOpts) {
        HILOGE("The third argument requires listener function");
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }

    auto ret = CopyCore::DoCopy(src, dest, opts);
    if (!ret.IsSuccess()) {
        HILOGE("DoCopy failed!");
        const FsError &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}
} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS