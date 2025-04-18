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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_SIGNATURE_H
#define INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_SIGNATURE_H

#pragma once

#include <string>
#include <ani_signature_builder.h>

namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature {
using namespace std;
using namespace arkts::ani_signature;

struct BasicTypes {
    static const Type undefinedType;
    static const Type nullType;
    static const Type booleanType;
    static const Type byteType;
    static const Type charType;
    static const Type shortType;
    static const Type intType;
    static const Type longType;
    static const Type floatType;
    static const Type doubleType;
};

struct BaseType {
    static const string ctorDesc;
    static const string ctorSig0;
};

namespace BoxedTypes {

struct Boolean : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string unboxedDesc;
    static const string unboxedSig;
};

struct Int : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct Double : public BaseType {
    static const Type classType;
    static const string classDesc;
};

} // namespace BoxedTypes

namespace BuiltInTypes {

struct Object : public BaseType {
    static const Type classType;
    static const string classDesc;
};

inline const Type &objectType = BuiltInTypes::Object::classType;

struct String : public BaseType {
    static const Type classType;
    static const string classDesc;
};

inline const Type &stringType = BuiltInTypes::String::classType;

struct Array : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
    static const string getterDesc;
    static const string setterDesc;
    static const string objectGetterSig;
    static const string objectSetterSig;
};

struct ArrayBuffer : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct BigInt : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct BusinessError : public BaseType {
    static const Type classType;
    static const string classDesc;
};

}; // namespace BuiltInTypes

namespace FS {

struct ConflictFilesInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct FileInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct ProgressInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct RandomAccessFileInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct ReaderIteratorInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct ReaderIteratorResultInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct StatInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct StreamInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct TaskSignal : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct WatcherInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

struct WatchEventInner : public BaseType {
    static const Type classType;
    static const string classDesc;
    static const string ctorSig;
};

} // namespace FS

namespace Impl {

struct EnvironmentImpl : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct FileIoImpl : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct HashImpl : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct SecurityLabelImpl : public BaseType {
    static const Type classType;
    static const string classDesc;
};

struct StatvfsImpl : public BaseType {
    static const Type classType;
    static const string classDesc;
};

} // namespace Impl

} // namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature

#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_SIGNATURE_H