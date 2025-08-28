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

#include "hashstream_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "hs_hashstream.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
namespace fs = std::filesystem;
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

HsHashStream *Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap hashstream err: %{private}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    HsHashStream *hashStream = reinterpret_cast<HsHashStream *>(ptrValue);
    return hashStream;
}

void HashStreamAni::Update(ani_env *env, [[maybe_unused]] ani_object object, ani_arraybuffer buffer)
{
    auto hashStream = Unwrap(env, object);
    if (hashStream == nullptr) {
        HILOGE("Cannot unwrap hashStream!");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto [succ, arrayBuffer] = TypeConverter::ToArrayBuffer(env, buffer);
    if (!succ) {
        HILOGE("illegal array buffer");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = hashStream->Update(arrayBuffer);
    if (!ret.IsSuccess()) {
        HILOGE("Cannot Update!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_string HashStreamAni::Digest(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto hashStream = Unwrap(env, object);
    if (hashStream == nullptr) {
        HILOGE("Cannot unwrap hashStream!");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto ret = hashStream->Digest();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot Digest!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const auto &res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Convert result to ani string failed");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

void CallBindNativePtr(ani_env *env, ani_object obj, HsHashStream *hsHashStream)
{
    auto bindNativePtrSig = HASH::HashStreamImpl::bindNativePtrSig.c_str();
    ani_long longValue = reinterpret_cast<ani_long>(hsHashStream);
    ani_status ret = env->Object_CallMethodByName_Void(obj, "bindNativePtr", bindNativePtrSig, longValue);
    if (ret != ANI_OK) {
        HILOGE("Object_CallMethodByName_Void failed. ret = %{public}d", static_cast<int32_t>(ret));
        return;
    }
}

void HashStreamAni::Constructor(ani_env *env, ani_object obj, ani_string alg)
{
    auto [succ, algorithm] = TypeConverter::ToUTF8String(env, alg);
    if (!succ) {
        HILOGE("Invalid alg");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto ret = HsHashStream::Constructor(algorithm);
    if (!ret.IsSuccess()) {
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(ret.GetData().value()));
    if (ANI_OK != AniHelper::SetFieldValue(env, obj, "nativePtr", ptr)) {
        HILOGE("Failed to wrap entity for obj HashStream");
        ErrorHandler::Throw(env, EIO);
        return;
    }

    CallBindNativePtr(env, obj, ret.GetData().value());
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS