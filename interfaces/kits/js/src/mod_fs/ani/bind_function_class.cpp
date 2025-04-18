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

#include <array>

#include <ani.h>

#include "access_ani.h"
#include "ani_signature.h"
#include "bind_function.h"
#include "close_ani.h"
#include "connectdfs_ani.h"
#include "copy_ani.h"
#include "copy_dir_ani.h"
#include "copy_file_ani.h"
#include "create_randomaccessfile_ani.h"
#include "create_stream_ani.h"
#include "disconnectdfs_ani.h"
#include "dup_ani.h"
#include "fdatasync_ani.h"
#include "fdopen_stream_ani.h"
#include "file_ani.h"
#include "filemgmt_libhilog.h"
#include "fsync_ani.h"
#include "fs_watcher_ani.h"
#include "listfile_ani.h"
#include "lseek_ani.h"
#include "lstat_ani.h"
#include "mkdir_ani.h"
#include "mkdtemp_ani.h"
#include "move_ani.h"
#include "movedir_ani.h"
#include "open_ani.h"
#include "randomaccessfile_ani.h"
#include "read_ani.h"
#include "read_lines_ani.h"
#include "read_text_ani.h"
#include "reader_iterator_ani.h"
#include "rename_ani.h"
#include "rmdir_ani.h"
#include "stat_ani.h"
#include "stream_ani.h"
#include "symlink_ani.h"
#include "task_signal_ani.h"
#include "truncate_ani.h"
#include "unlink_ani.h"
#include "utimes_ani.h"
#include "watcher_ani.h"
#include "write_ani.h"
#include "xattr_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static ani_status BindRafFileMethods(ani_env *env)
{
    auto classDesc = FS::RandomAccessFileInner::classDesc.c_str();

    std::array methods = {
        ani_native_function {
            "setFilePointer0", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::SetFilePointer) },
        ani_native_function { "close", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::Close) },
        ani_native_function { "writeSync0", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::WriteSync) },
        ani_native_function { "readSync0", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::ReadSync) },
        ani_native_function { "getReadStream", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::GetReadStream) },
        ani_native_function {
            "getWriteStream", nullptr, reinterpret_cast<void *>(RandomAccessFileAni::GetWriteStream) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindWatcherClassMethods(ani_env *env)
{
    auto classDesc = FS::WatcherInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "start", nullptr, reinterpret_cast<void *>(FsWatcherAni::Start) },
        ani_native_function { "stop", nullptr, reinterpret_cast<void *>(FsWatcherAni::Stop) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindFileMethods(ani_env *env)
{
    auto classDesc = FS::FileInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "getParent", nullptr, reinterpret_cast<void *>(FileAni::GetParent) },
        ani_native_function { "lockSync", nullptr, reinterpret_cast<void *>(FileAni::LockSync) },
        ani_native_function { "tryLock", nullptr, reinterpret_cast<void *>(FileAni::TryLock) },
        ani_native_function { "unlock", nullptr, reinterpret_cast<void *>(FileAni::UnLock) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindReaderIteratorMethods(ani_env *env)
{
    auto classDesc = FS::ReaderIteratorInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "next", nullptr, reinterpret_cast<void *>(ReaderIteratorAni::Next) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindStatClassMethods(ani_env *env)
{
    auto classDesc = FS::StatInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "isBlockDevice", nullptr, reinterpret_cast<void *>(StatAni::IsBlockDevice) },
        ani_native_function { "isCharacterDevice", nullptr, reinterpret_cast<void *>(StatAni::IsCharacterDevice) },
        ani_native_function { "isDirectory", nullptr, reinterpret_cast<void *>(StatAni::IsDirectory) },
        ani_native_function { "isFIFO", nullptr, reinterpret_cast<void *>(StatAni::IsFIFO) },
        ani_native_function { "isFile", nullptr, reinterpret_cast<void *>(StatAni::IsFile) },
        ani_native_function { "isSocket", nullptr, reinterpret_cast<void *>(StatAni::IsSocket) },
        ani_native_function { "isSymbolicLink", nullptr, reinterpret_cast<void *>(StatAni::IsSymbolicLink) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindStreamMethods(ani_env *env)
{
    auto classDesc = FS::StreamInner::classDesc.c_str();

    std::array methods = {
        ani_native_function { "closeSync", nullptr, reinterpret_cast<void *>(StreamAni::Close) },
        ani_native_function { "flushSync", nullptr, reinterpret_cast<void *>(StreamAni::Flush) },
        ani_native_function { "readSync", nullptr, reinterpret_cast<void *>(StreamAni::Read) },
        ani_native_function { "writeSync", nullptr, reinterpret_cast<void *>(StreamAni::Write) },
        ani_native_function { "seek", nullptr, reinterpret_cast<void *>(StreamAni::Seek) },
    };

    return BindClass(env, classDesc, methods);
}

static ani_status BindTaskSignalClassMethods(ani_env *env)
{
    auto classDesc = FS::TaskSignal::classDesc.c_str();

    std::array methods = {
        ani_native_function { "cancel", nullptr, reinterpret_cast<void *>(TaskSignalAni::Cancel) },
        ani_native_function { "onCancelNative", nullptr, reinterpret_cast<void *>(TaskSignalAni::OnCancel) },
    };

    return BindClass(env, classDesc, methods);
}

const static string mkdirCtorSig0 = Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType });
const static string mkdirCtorSig1 =
    Builder::BuildSignatureDescriptor({ BuiltInTypes::stringType, BasicTypes::booleanType });

static ani_status BindStaticMethods(ani_env *env)
{
    auto classDesc = Impl::FileIoImpl::classDesc.c_str();

    std::array methods = {
        ani_native_function { "closeSync", nullptr, reinterpret_cast<void *>(CloseAni::CloseSync) },
        ani_native_function { "connectDfs", nullptr, reinterpret_cast<void *>(ConnectDfsAni::ConnectDfsSync) },
        ani_native_function { "copyDirSync", nullptr, reinterpret_cast<void *>(CopyDirAni::CopyDirSync) },
        ani_native_function { "copyFileSync", nullptr, reinterpret_cast<void *>(CopyFileAni::CopyFileSync) },
        ani_native_function { "copySync", nullptr, reinterpret_cast<void *>(CopyAni::CopySync) },
        ani_native_function { "createRandomAccessFileSync", nullptr,
            reinterpret_cast<void *>(CreateRandomAccessFileAni::CreateRandomAccessFileSync) },
        ani_native_function {
            "createStreamSync", nullptr, reinterpret_cast<void *>(CreateStreamAni::CreateStreamSync) },
        ani_native_function { "createWatcherSync", nullptr, reinterpret_cast<void *>(WatcherAni::CreateWatcherSync) },
        ani_native_function { "disConnectDfs", nullptr, reinterpret_cast<void *>(DisConnectDfsAni::DisConnectDfsSync) },
        ani_native_function { "doAccessSync", nullptr, reinterpret_cast<void *>(AccessAni::AccessSync3) },
        ani_native_function { "dup", nullptr, reinterpret_cast<void *>(DupAni::Dup) },
        ani_native_function { "fdatasyncSync", nullptr, reinterpret_cast<void *>(FDataSyncAni::FDataSyncSync) },
        ani_native_function {
            "fdopenStreamSync", nullptr, reinterpret_cast<void *>(FdopenStreamAni::FdopenStreamSync) },
        ani_native_function { "fsyncSync", nullptr, reinterpret_cast<void *>(FsyncAni::FsyncSync) },
        ani_native_function { "getxattrSync", nullptr, reinterpret_cast<void *>(XattrAni::GetXattrSync) },
        ani_native_function { "listFileSync", nullptr, reinterpret_cast<void *>(ListFileAni::ListFileSync) },
        ani_native_function { "lseekSync", nullptr, reinterpret_cast<void *>(LseekAni::LseekSync) },
        ani_native_function { "lstatSync", nullptr, reinterpret_cast<void *>(LstatAni::LstatSync) },
        ani_native_function { "mkdirSync", mkdirCtorSig0.c_str(), reinterpret_cast<void *>(MkdirkAni::MkdirSync0) },
        ani_native_function { "mkdirSync", mkdirCtorSig1.c_str(), reinterpret_cast<void *>(MkdirkAni::MkdirSync1) },
        ani_native_function { "mkdtempSync", nullptr, reinterpret_cast<void *>(MkdtempAni::MkdtempSync) },
        ani_native_function { "movedirSync", nullptr, reinterpret_cast<void *>(MoveDirAni::MoveDirSync) },
        ani_native_function { "moveFileSync", nullptr, reinterpret_cast<void *>(MoveAni::MoveFileSync) },
        ani_native_function { "openSync", nullptr, reinterpret_cast<void *>(OpenAni::OpenSync) },
        ani_native_function { "readlinesSync", nullptr, reinterpret_cast<void *>(ReadLinesAni::ReadLinesSync) },
        ani_native_function { "readSync", nullptr, reinterpret_cast<void *>(ReadAni::ReadSync) },
        ani_native_function { "readTextSync", nullptr, reinterpret_cast<void *>(ReadTextAni::ReadTextSync) },
        ani_native_function { "renameSync", nullptr, reinterpret_cast<void *>(RenameAni::RenameSync) },
        ani_native_function { "rmdirSync", nullptr, reinterpret_cast<void *>(RmdirAni::RmdirSync) },
        ani_native_function { "setxattrSync", nullptr, reinterpret_cast<void *>(XattrAni::SetXattrSync) },
        ani_native_function { "statSync", nullptr, reinterpret_cast<void *>(StatAni::StatSync) },
        ani_native_function { "symlinkSync", nullptr, reinterpret_cast<void *>(SymlinkAni::SymlinkSync) },
        ani_native_function { "truncateSync", nullptr, reinterpret_cast<void *>(TruncateAni::TruncateSync) },
        ani_native_function { "unlinkSync", nullptr, reinterpret_cast<void *>(UnlinkAni::UnlinkSync) },
        ani_native_function { "utimes", nullptr, reinterpret_cast<void *>(UtimesAni::Utimes) },
        ani_native_function { "writeSync", nullptr, reinterpret_cast<void *>(WriteAni::WriteSync) },
    };
    return BindClass(env, classDesc, methods);
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr) {
        HILOGE("Invalid parameter vm");
        return ANI_INVALID_ARGS;
    }

    if (result == nullptr) {
        HILOGE("Invalid parameter result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        HILOGE("Invalid ani version!");
        return ANI_INVALID_VERSION;
    }

    if ((status = BindStaticMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native static methods for BindStaticMethods!");
        return status;
    };

    if ((status = BindFileMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for file Class");
        return status;
    };

    if ((status = BindReaderIteratorMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for ReaderIterator Class");
        return status;
    };

    if ((status = BindStatClassMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for Stat Class!");
        return status;
    };

    if ((status = BindRafFileMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for RafFile Class");
        return status;
    };

    if ((status = BindStreamMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for Stream Class!");
        return status;
    };

    if ((status = BindTaskSignalClassMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for TaskSignal Class!");
        return status;
    };

    if ((status = BindWatcherClassMethods(env)) != ANI_OK) {
        HILOGE("Cannot bind native methods for Watcher Class");
        return status;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
