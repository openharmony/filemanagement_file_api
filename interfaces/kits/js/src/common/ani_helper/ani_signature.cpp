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

#include "ani_signature.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature {
// BasicTypes
const Type BasicTypes::undefinedType = Builder::BuildUndefined();
const Type BasicTypes::nullType = Builder::BuildNull();
const Type BasicTypes::booleanType = Builder::BuildBoolean();
const Type BasicTypes::byteType = Builder::BuildByte();
const Type BasicTypes::charType = Builder::BuildChar();
const Type BasicTypes::shortType = Builder::BuildShort();
const Type BasicTypes::intType = Builder::BuildInt();
const Type BasicTypes::longType = Builder::BuildLong();
const Type BasicTypes::floatType = Builder::BuildFloat();
const Type BasicTypes::doubleType = Builder::BuildDouble();
// BasicTypesConverter::toLong
const string BasicTypesConverter::toLong = "toLong";
const string BasicTypesConverter::toInt = "toInt";
const string BasicTypesConverter::toDouble = "toDouble";

// BoxedTypes::Boolean
const Type BoxedTypes::Boolean::classType = Builder::BuildClass("std.core.Boolean");
const string BoxedTypes::Boolean::classDesc = BoxedTypes::Double::classType.Descriptor();
const string BoxedTypes::Boolean::unboxedDesc = "unboxed";
const string BoxedTypes::Boolean::unboxedSig = Builder::BuildSignatureDescriptor({}, BasicTypes::booleanType);
// BoxedTypes::Int
const Type BoxedTypes::Int::classType = Builder::BuildClass("std.core.Int");
const string BoxedTypes::Int::classDesc = BoxedTypes::Int::classType.Descriptor();
const string BoxedTypes::Int::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::intType });
// BoxedTypes::Long
const Type BoxedTypes::Long::classType = Builder::BuildClass("std.core.Long");
const string BoxedTypes::Long::classDesc = BoxedTypes::Long::classType.Descriptor();
const string BoxedTypes::Long::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// BoxedTypes::Double
const Type BoxedTypes::Double::classType = Builder::BuildClass("std.core.Double");
const string BoxedTypes::Double::classDesc = BoxedTypes::Double::classType.Descriptor();
// BaseType
const string BaseType::ctorDesc = Builder::BuildConstructorName();
const string BaseType::ctorSig0 = Builder::BuildSignatureDescriptor({});
// BuiltInTypes::Object
const Type BuiltInTypes::Object::classType = Builder::BuildClass("std.core.Object");
const string BuiltInTypes::Object::classDesc = BuiltInTypes::Object::classType.Descriptor();
// BuiltInTypes::String
const Type BuiltInTypes::String::classType = Builder::BuildClass("std.core.String");
const string BuiltInTypes::String::classDesc = BuiltInTypes::String::classType.Descriptor();
// BuiltInTypes::Array
const Type BuiltInTypes::Array::classType = Builder::BuildClass("escompat.Array");
const string BuiltInTypes::Array::classDesc = BuiltInTypes::Array::classType.Descriptor();
const string BuiltInTypes::Array::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::intType });
const string BuiltInTypes::Array::getterDesc = "$_get";
const string BuiltInTypes::Array::setterDesc = "$_set";
const string BuiltInTypes::Array::objectGetterSig =
    Builder::BuildSignatureDescriptor({ BasicTypes::intType }, BuiltInTypes::objectType);
const string BuiltInTypes::Array::objectSetterSig =
    Builder::BuildSignatureDescriptor({ BasicTypes::intType, BuiltInTypes::objectType });
// BuiltInTypes::ArrayBuffer
const Type BuiltInTypes::ArrayBuffer::classType = Builder::BuildClass("escompat.ArrayBuffer");
const string BuiltInTypes::ArrayBuffer::classDesc = BuiltInTypes::ArrayBuffer::classType.Descriptor();
const string BuiltInTypes::ArrayBuffer::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::intType });
// BuiltInTypes::BigInt
const Type BuiltInTypes::BigInt::classType = Builder::BuildClass("escompat.BigInt");
const string BuiltInTypes::BigInt::classDesc = BuiltInTypes::BigInt::classType.Descriptor();
const string BuiltInTypes::BigInt::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// BuiltInTypes::BusinessError
const Type BuiltInTypes::BusinessError::classType = Builder::BuildClass("@ohos.base.BusinessError");
const string BuiltInTypes::BusinessError::classDesc = BuiltInTypes::BusinessError::classType.Descriptor();
// FS::ConflictFilesInner
const Type FS::ConflictFilesInner::classType = Builder::BuildClass("@ohos.file.fs.ConflictFilesInner");
const string FS::ConflictFilesInner::classDesc = FS::ConflictFilesInner::classType.Descriptor();
const string FS::ConflictFilesInner::ctorSig =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, BuiltInTypes::stringType });
// FS::FileInner
const Type FS::FileInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.FileInner");
const string FS::FileInner::classDesc = FS::FileInner::classType.Descriptor();
const string FS::FileInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::ProgressInner
const Type FS::ProgressInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.ProgressInner");
const string FS::ProgressInner::classDesc = FS::ProgressInner::classType.Descriptor();
const string FS::ProgressInner::ctorSig =
    Builder::BuildSignatureDescriptor({ BasicTypes::longType, BasicTypes::longType });
// FS::RandomAccessFileInner
const Type FS::RandomAccessFileInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.RandomAccessFileInner");
const string FS::RandomAccessFileInner::classDesc = FS::RandomAccessFileInner::classType.Descriptor();
const string FS::RandomAccessFileInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::ReaderIteratorInner
const Type FS::ReaderIteratorInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.ReaderIteratorInner");
const string FS::ReaderIteratorInner::classDesc = FS::ReaderIteratorInner::classType.Descriptor();
const string FS::ReaderIteratorInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::ReaderIteratorResultInner
const Type FS::ReaderIteratorResultInner::classType =
    Builder::BuildClass("@ohos.file.fs.fileIo.ReaderIteratorResultInner");
const string FS::ReaderIteratorResultInner::classDesc = FS::ReaderIteratorResultInner::classType.Descriptor();
const string FS::ReaderIteratorResultInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::StatInner
const Type FS::StatInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.StatInner");
const string FS::StatInner::classDesc = FS::StatInner::classType.Descriptor();
const string FS::StatInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::StreamInner
const Type FS::StreamInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.StreamInner");
const string FS::StreamInner::classDesc = FS::StreamInner::classType.Descriptor();
const string FS::StreamInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
// FS::TaskSignal
const Type FS::TaskSignal::classType = Builder::BuildClass("@ohos.file.fs.fileIo.TaskSignal");
const string FS::TaskSignal::classDesc = FS::TaskSignal::classType.Descriptor();
const string FS::TaskSignal::ctorDesc = Builder::BuildConstructorName();
const string FS::TaskSignal::ctorSig = Builder::BuildSignatureDescriptor({});
const string FS::TaskSignal::nativeTaskSignal = "nativeTaskSignal";
// FS::WatcherInner
const Type FS::WatcherInner::classType = Builder::BuildClass("@ohos.file.fs.fileIo.WatcherInner");
const string FS::WatcherInner::classDesc = FS::WatcherInner::classType.Descriptor();
const string FS::WatcherInner::ctorSig = Builder::BuildSignatureDescriptor({ BasicTypes::longType });
const string FS::WatcherInner::nativePtr = "nativePtr";
// FS::WatchEventInner
const Type FS::WatchEventInner::classType = Builder::BuildClass("@ohos.file.fs.WatchEventInner");
const string FS::WatchEventInner::classDesc = FS::WatchEventInner::classType.Descriptor();
const string FS::WatchEventInner::ctorSig =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, BasicTypes::intType, BasicTypes::intType });
// FS::ReadStream
const Type FS::ReadStream::classType = Builder::BuildClass("@ohos.file.fs.fileIo.ReadStream");
const string FS::ReadStream::classDesc = FS::ReadStream::classType.Descriptor();
const string FS::ReadStream::ctorSig =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, FS::ReadStreamOptionsInner::classType });
// FS::WriteStream
const Type FS::WriteStream::classType = Builder::BuildClass("@ohos.file.fs.fileIo.WriteStream");
const string FS::WriteStream::classDesc = FS::WriteStream::classType.Descriptor();
const string FS::WriteStream::ctorSig =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, FS::WriteStreamOptionsInner::classType });
// FS::AtomicFile
const Type FS::AtomicFile::classType = Builder::BuildClass("@ohos.file.fs.fileIo.AtomicFile");
const string FS::AtomicFile::classDesc = FS::AtomicFile::classType.Descriptor();
const string FS::AtomicFile::ctorSig = Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType });
// FS::LocationType
const Type FS::LocationType::classType = Builder::BuildClass("@ohos.file.fs.fileIo.LocationType");
const string FS::LocationType::classDesc = FS::LocationType::classType.Descriptor();
// Impl::EnvironmentImpl
const Type Impl::EnvironmentImpl::classType = Builder::BuildClass("@ohos.file.environment.EnvironmentImpl");
const string Impl::EnvironmentImpl::classDesc = Impl::EnvironmentImpl::classType.Descriptor();
// Impl::FileIoImpl
const Type Impl::FileIoImpl::classType = Builder::BuildClass("@ohos.file.fs.FileIoImpl");
const string Impl::FileIoImpl::classDesc = Impl::FileIoImpl::classType.Descriptor();
// Impl::HashImpl
const Type Impl::HashImpl::classType = Builder::BuildClass("@ohos.file.hash.HashImpl");
const string Impl::HashImpl::classDesc = Impl::HashImpl::classType.Descriptor();
// Impl::SecurityLabelImpl
const Type Impl::SecurityLabelImpl::classType = Builder::BuildClass("@ohos.file.securityLabel.SecurityLabelImpl");
const string Impl::SecurityLabelImpl::classDesc = Impl::SecurityLabelImpl::classType.Descriptor();
// Impl::StatvfsImpl
const Type Impl::StatvfsImpl::classType = Builder::BuildClass("@ohos.file.statvfs.StatvfsImpl");
const string Impl::StatvfsImpl::classDesc = Impl::StatvfsImpl::classType.Descriptor();
// HASH::HashStreamImpl
const Type HASH::HashStreamImpl::classType = Builder::BuildClass("@ohos.file.hash.HashStreamImpl");
const string HASH::HashStreamImpl::classDesc = HASH::HashStreamImpl::classType.Descriptor();
const string HASH::HashStreamImpl::ctorSig = Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType });
// FS::ReadStreamOptionsInner
const Type FS::ReadStreamOptionsInner::classType = Builder::BuildClass("@ohos.file.fs.ReadStreamOptionsInner");
const string FS::ReadStreamOptionsInner::classDesc = FS::ReadStreamOptionsInner::classType.Descriptor();
const string FS::ReadStreamOptionsInner::ctorSig = Builder::BuildSignatureDescriptor({});
const string FS::ReadStreamOptionsInner::ctorDesc = Builder::BuildConstructorName();
// FS::WriteStreamOptionsInner
const Type FS::WriteStreamOptionsInner::classType = Builder::BuildClass("@ohos.file.fs.WriteStreamOptionsInner");
const string FS::WriteStreamOptionsInner::classDesc = FS::WriteStreamOptionsInner::classType.Descriptor();
const string FS::WriteStreamOptionsInner::ctorSig = Builder::BuildSignatureDescriptor({});
const string FS::WriteStreamOptionsInner::ctorDesc = Builder::BuildConstructorName();

} // namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature