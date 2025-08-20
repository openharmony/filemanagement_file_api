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

#include "reader_iterator_ani.h"

#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "reader_iterator_result_ani.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

ani_object ReaderIteratorAni::Wrap(ani_env *env, const FsReaderIterator *it)
{
    if (it == nullptr) {
        HILOGE("FsReaderIterator pointer is null!");
        return nullptr;
    }

    auto classDesc = FS::ReaderIteratorInner::classDesc.c_str();
    ani_class cls;
    if (ANI_OK != env->FindClass(classDesc, &cls)) {
        HILOGE("Cannot find class %s", classDesc);
        return nullptr;
    }

    auto ctorDesc = FS::ReaderIteratorInner::ctorDesc.c_str();
    auto ctorSig = FS::ReaderIteratorInner::ctorSig.c_str();
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, ctorDesc, ctorSig, &ctor)) {
        HILOGE("Cannot find constructor method for class %s", classDesc);
        return nullptr;
    }

    ani_long ptr = static_cast<ani_long>(reinterpret_cast<std::uintptr_t>(it));
    ani_object obj;
    if (ANI_OK != env->Object_New(cls, ctor, &obj, ptr)) {
        HILOGE("New %s obj Failed!", classDesc);
        return nullptr;
    }

    return obj;
}

FsReaderIterator *ReaderIteratorAni::Unwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        HILOGE("Unwrap fsReaderIterator err: %{private}d", ret);
        return nullptr;
    }
    uintptr_t ptrValue = static_cast<uintptr_t>(nativePtr);
    FsReaderIterator *readeriterator = reinterpret_cast<FsReaderIterator *>(ptrValue);
    return readeriterator;
}

ani_object ReaderIteratorAni::Next(ani_env *env, [[maybe_unused]] ani_object object)
{
    auto fsReaderIterator = Unwrap(env, object);
    if (fsReaderIterator == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    auto ret = fsReaderIterator->Next();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot get readeriterator next!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto nextRet = ret.GetData().value();
    auto result = ReaderIteratorResultAni::Wrap(env, nextRet);
    if (result == nullptr) {
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS