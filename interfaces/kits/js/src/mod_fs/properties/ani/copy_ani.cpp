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
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static void SetProgressListenerCb(ani_env *env, ani_ref &callback, CopyOptions &opts)
{
    ani_vm *vm = nullptr;
    env->GetVM(&vm);

    opts.listenerCb = [vm, &callback](uint64_t progressSize, uint64_t totalSize) -> void {
        ani_status ret;
        ani_object progress = {};
        auto classDesc = FS::ProgressInner::classDesc.c_str();
        ani_class cls;

        auto env = AniHelper::GetThreadEnv(vm);
        if (env == nullptr) {
            HILOGE("failed to GetThreadEnv");
            return;
        }

        if (progressSize > MAX_VALUE || totalSize > MAX_VALUE) {
            HILOGE("progressSize or totalSize exceed MAX_VALUE: %{private}" PRIu64 " %{private}" PRIu64, progressSize,
                totalSize);
        }

        if ((ret = env->FindClass(classDesc, &cls)) != ANI_OK) {
            HILOGE("Not found %{private}s, err: %{private}d", classDesc, ret);
            return;
        }

        auto ctorDesc = FS::ProgressInner::ctorDesc.c_str();
        auto ctorSig = FS::ProgressInner::ctorSig.c_str();
        ani_method ctor;
        if ((ret = env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) != ANI_OK) {
            HILOGE("Not found ctor, err: %{private}d", ret);
            return;
        }

        if ((ret = env->Object_New(cls, ctor, &progress, ani_double(static_cast<double>(progressSize)),
            ani_double(static_cast<double>(totalSize)))) != ANI_OK) {
            HILOGE("New ProgressInner Fail, err: %{private}d", ret);
            return;
        }

        std::vector<ani_ref> vec;
        vec.push_back(progress);
        ani_ref result;
        ret = env->FunctionalObject_Call(static_cast<ani_fn_object>(callback), vec.size(), vec.data(), &result);
        if (ret != ANI_OK) {
            HILOGE("FunctionalObject_Call, err: %{private}d", ret);
            return;
        }
    };
}

static bool SetTaskSignal(ani_env *env, ani_ref &copySignal, CopyOptions &opts)
{
    ani_status ret;
    auto taskSignalEntityCore = CreateSharedPtr<TaskSignalEntityCore>();
    if (taskSignalEntityCore == nullptr) {
        HILOGE("Failed to request heap memory.");
        return false;
    }

    ret = env->Object_SetFieldByName_Long(static_cast<ani_object>(copySignal), "nativeTaskSignal",
        reinterpret_cast<ani_long>(taskSignalEntityCore.get()));
    if (ret != ANI_OK) {
        HILOGE("Object set nativeTaskSignal err: %{private}d", ret);
        return false;
    }

    taskSignalEntityCore->taskSignal_ = std::make_shared<TaskSignal>();
    opts.taskSignalEntityCore = move(taskSignalEntityCore);

    return true;
}

static tuple<bool, optional<CopyOptions>> ParseOptions(ani_env *env, ani_ref &cb, ani_object &options)
{
    ani_boolean isUndefined;
    ani_status ret;
    env->Reference_IsUndefined(options, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    CopyOptions opts;
    ani_ref prog;
    if ((ret = env->Object_GetPropertyByName_Ref(options, "progressListener", &prog)) != ANI_OK) {
        HILOGE("Object_GetPropertyByName_Ref progressListener, err: %{private}d", ret);
        return { false, nullopt };
    }
    env->Reference_IsUndefined(prog, &isUndefined);
    if (!isUndefined) {
        env->GlobalReference_Create(prog, &cb);
        SetProgressListenerCb(env, cb, opts);
    }

    ani_ref signal;
    if ((ret = env->Object_GetPropertyByName_Ref(options, "copySignal", &signal)) != ANI_OK) {
        HILOGE("Object_GetPropertyByName_Ref copySignal, err: %{private}d", ret);
        return { false, nullopt };
    }
    env->Reference_IsUndefined(signal, &isUndefined);
    if (!isUndefined) {
        if (!SetTaskSignal(env, signal, opts)) {
            return { false, nullopt };
        }
    }

    return { true, make_optional(move(opts)) };
}

void CopyAni::CopySync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string srcUri, ani_string destUri, ani_object options)
{
    auto [succSrc, src] = TypeConverter::ToUTF8String(env, srcUri);
    auto [succDest, dest] = TypeConverter::ToUTF8String(env, destUri);
    if (!succSrc || !succDest) {
        HILOGE("The first/second argument requires filepath");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    ani_ref cb;
    auto [succOpts, opts] = ParseOptions(env, cb, options);
    if (!succOpts) {
        HILOGE("Failed to parse opts");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = CopyCore::DoCopy(src, dest, opts);
    if (opts.has_value() && opts->listenerCb != nullptr) {
        env->GlobalReference_Delete(cb);
    }
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