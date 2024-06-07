/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "file_fs_ffi.h"
#include "copy_file.h"
#include "fdatasync.h"
#include "fsync.h"
#include "list_file.h"
#include "lseek.h"
#include "macro.h"
#include "mkdtemp.h"
#include "move_file.h"
#include "symlink.h"
#include "uni_error.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

extern "C" {
RetDataI64 FfiOHOSFileFsStatByID(int32_t file)
{
    LOGI("FS_TEST::FfiOHOSFileFsStatByID");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStat] = FileFsImpl::Stat(file);
    if (nativeStat == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsStatByID error");
        ret.code = state;
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsStatByID success");
    ret.code = state;
    ret.data = nativeStat->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsStatByString(const char* file)
{
    LOGI("FS_TEST::FfiOHOSFileFsStatByString");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStat] = FileFsImpl::Stat(file);
    if (nativeStat == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsStatByString error");
        ret.code = state;
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsStatByString success");
    ret.code = state;
    ret.data = nativeStat->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsCreateStream(const char* path, const char* mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCreateStream");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStream] = FileFsImpl::CreateStream(path, mode);
    ret.code = state;
    if (nativeStream == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsCreateStream error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsCreateStream success");
    ret.data = nativeStream->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsFdopenStream(int32_t fd, const char* mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsFdopenStream");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStream] = FileFsImpl::FdopenStream(fd, mode);
    ret.code = state;
    if (nativeStream == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsFdopenStream error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsFdopenStream success");
    ret.data = nativeStream->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsLstat(const char* path)
{
    LOGI("FS_TEST::FfiOHOSFileFsLstat");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeStat] = FileFsImpl::Lstat(path);
    ret.code = state;
    if (nativeStat == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsLstat error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsLstat success");
    ret.data = nativeStat->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsRead(int32_t fd, char* buffer, int64_t bufLen, size_t length, int64_t offset)
{
    LOGI("FS_TEST::FfiOHOSFileFsRead");
    auto ret = FileFsImpl::Read(fd, buffer, bufLen, length, offset);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsRead success");
    return ret;
}

RetDataI64 FfiOHOSFileFsReadCur(int32_t fd, char* buffer, int64_t bufLen, size_t length)
{
    LOGI("FS_TEST::FfiOHOSFileFsReadCur");
    auto ret = FileFsImpl::ReadCur(fd, buffer, bufLen, length);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsReadCur success");
    return ret;
}

RetDataI64 FfiOHOSFileFsWrite(int32_t fd, char* buffer, size_t length, int64_t offset, const char* encode)
{
    LOGI("FS_TEST::FfiOHOSFileFsWrite");
    auto ret = FileFsImpl::Write(fd, buffer, length, offset, encode);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsWrite success");
    return ret;
}

RetDataI64 FfiOHOSFileFsWriteCur(int32_t fd, char* buffer, size_t length, const char* encode)
{
    LOGI("FS_TEST::FfiOHOSFileFsWriteCur");
    auto ret = FileFsImpl::WriteCur(fd, buffer, length, encode);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsWriteCur success");
    return ret;
}

RetDataI64 FfiOHOSFileFsCreateRandomAccessFileByString(const char* file, int64_t mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByString");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeCreateRandomAccessFile] = FileFsImpl::CreateRandomAccessFileSync(file, mode);
    ret.code = state;
    if (nativeCreateRandomAccessFile == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByString error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByString success");
    ret.data = nativeCreateRandomAccessFile->GetID();
    return ret;
}

RetDataI64 FfiOHOSFileFsCreateRandomAccessFileByID(int64_t file, int64_t mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByID");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<FileEntity>(file);
    auto [state, nativeCreateRandomAccessFile] = FileFsImpl::CreateRandomAccessFileSync(instance, mode);
    ret.code = state;
    if (nativeCreateRandomAccessFile == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByID error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFileByID success");
    ret.data = nativeCreateRandomAccessFile->GetID();
    return ret;
}

int32_t FfiOHOSRandomAccessFileGetFd(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileGetFd");
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileGetFd instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileGetFd success");
    return instance->GetFd();
}

int64_t FfiOHOSRandomAccessFileGetFPointer(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileGetFPointer");
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileGetFPointer instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileGetFPointer success");
    return instance->GetFPointer();
}

void FfiOHOSRandomAccessFileSetFilePointerSync(int64_t id, int64_t fd)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileSetFilePointerSync");
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileSetFilePointerSync instance not exist %{public}" PRId64, id);
        return;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileSetFilePointerSync success");
    instance->SetFilePointerSync(fd);
    return;
}

void FfiOHOSRandomAccessFileClose(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileClose");
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileClose instance not exist %{public}" PRId64, id);
        return;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileClose success");
    return instance->CloseSync();
}

RetDataI64 FfiOHOSRandomAccessFileWrite(int64_t id, char* buf, size_t len, int64_t offset)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileWriteByString");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileWriteByString instance not exist %{public}" PRId64, id);
        ret.code = ERR_INVALID_INSTANCE_CODE;
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileWriteByString success");
    auto [code, data] = instance->WriteSync(buf, len, offset);
    ret.code = code;
    ret.data = data;
    return ret;
}

RetDataI64 FfiOHOSRandomAccessFileRead(int64_t id, char* buf, size_t len, int64_t offset)
{
    LOGI("FS_TEST::FfiOHOSRandomAccessFileReadByArray");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto instance = FFIData::GetData<RandomAccessFileImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSRandomAccessFileReadByArray instance not exist %{public}" PRId64, id);
        ret.code = ERR_INVALID_INSTANCE_CODE;
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSRandomAccessFileReadByArray success");
    auto [code, data] = instance->ReadSync(buf, len, offset);
    ret.code = code;
    ret.data = data;
    return ret;
}

int32_t FfiOHOSFileFsMkdir(const char* path, bool recursion, bool isTwoArgs)
{
    LOGI("FS_TEST::FfiOHOSFileFsMkdir");
    auto code = FileFsImpl::Mkdir(path, recursion, isTwoArgs);
    LOGI("FS_TEST::FfiOHOSFileFsMkdir success");
    return code;
}

int32_t FfiOHOSFileFsRmdir(const char* path)
{
    LOGI("FS_TEST::FfiOHOSFileFsRmdir");
    auto code = FileFsImpl::Rmdir(path);
    LOGI("FS_TEST::FfiOHOSFileFsRmdir success");
    return code;
}
 
RetDataCArrConflictFiles FfiOHOSFileFsMoveDir(const char* src, const char* dest, int32_t mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsMovedir");
    auto ret = FileFsImpl::MoveDir(src, dest, mode);
    LOGI("FS_TEST::FfiOHOSFileFsMovedir success");
    return ret;
}
 
int32_t FfiOHOSFileFsRename(const char* oldFile, const char* newFile)
{
    LOGI("FS_TEST::FfiOHOSFileFsRename");
    auto code = FileFsImpl::Rename(oldFile, newFile);
    LOGI("FS_TEST::FfiOHOSFileFsRename success");
    return code;
}

int32_t FfiOHOSFileFsUnlink(const char* path)
{
    LOGI("FS_TEST::FfiOHOSFileFsUnlink");
    auto code = FileFsImpl::Unlink(path);
    LOGI("FS_TEST::FfiOHOSFileFsUnlink success");
    return code;
}

RetDataCArrConflictFiles FfiOHOSFileFsCopyDir(const char* src, const char* dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCopyDir start");
    auto ret = CopyDirImpl::CopyDir(src, dest, mode);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
    }
    LOGI("FS_TEST::FfiOHOSFileFsCopyDir success");
    return ret;
}

int FfiOHOSFileFsCopyFile(const char* src, const char* dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCopyFile start");
    auto ret = CopyFileImpl::CopyFile(src, dest, mode);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsCopyFile success");
    return ret;
}

int FfiOHOSFileFsCopyFileSI(const char* src, int32_t dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileSI start");
    auto ret = CopyFileImpl::CopyFile(src, dest, mode);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileSI success");
    return ret;
}

int FfiOHOSFileFsCopyFileIS(int32_t src, const char* dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileIS start");
    auto ret = CopyFileImpl::CopyFile(src, dest, mode);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileIS success");
    return ret;
}

int FfiOHOSFileFsCopyFileII(int32_t src, int32_t dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileII start");
    auto ret = CopyFileImpl::CopyFile(src, dest, mode);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsCopyFileII success");
    return ret;
}

int FfiOHOSFileFsMoveFile(const char* src, const char* dest, int mode)
{
    LOGI("FS_TEST::FfiOHOSFileFsMoveFile start");
    auto ret = MoveFileImpl::MoveFile(src, dest, mode);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsMoveFile success");
    return ret;
}

RetDataCString FfiOHOSFileFsMkdtemp(const char* prefix)
{
    LOGI("FS_TEST::FfiOHOSFileFsMkdtemp start");
    auto ret = MkdtempImpl::Mkdtemp(prefix);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
    }
    LOGI("FS_TEST::FfiOHOSFileFsMkdtemp end");
    return ret;
}

RetDataBool FfiOHOSFileFsAccess(const char* path)
{
    LOGI("FS_TEST::FfiOHOSFileFsAccess");
    RetDataBool ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [status, accessStatus] = FileFsImpl::Access(path);
    ret.code = status;
    if (status != SUCCESS_CODE) {
        LOGI("FS_TEST::FfiOHOSFileFsAccess error");
        ret.data = false;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsAccess success");
    ret.data = accessStatus;
    return ret;
}

int32_t FfiOHOSFileFsTruncateByString(const char* file, int64_t len)
{
    LOGI("FS_TEST::FfiOHOSFileFsTruncateByString");
    auto code = FileFsImpl::Truncate(file, len);
    LOGI("FS_TEST::FfiOHOSFileFsTruncateByString success");
    return code;
}

int32_t FfiOHOSFileFsTruncateByFd(int32_t file, int64_t len)
{
    LOGI("FS_TEST::FfiOHOSFileFsTruncateByfd");
    auto code = FileFsImpl::Truncate(file, len);
    LOGI("FS_TEST::FfiOHOSFileFsTruncateByfd success");
    return code;
}

RetDataI64 FfiOHOSFileFsReadLines(char* path, char* encoding)
{
    LOGI("FS_TEST::FfiOHOSFileFsReadLine");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeReaderIterator] = FileFsImpl::ReadLines(path, encoding);
    ret.code = state;
    if (nativeReaderIterator == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsReadLine error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsReadLine success");
    ret.data = nativeReaderIterator->GetID();
    return ret;
}

RetReaderIteratorResult FfiOHOSFileFsReaderIteratorNext(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFileFsReaderIteratorNext");
    RetReaderIteratorResult ret = { .code = ERR_INVALID_INSTANCE_CODE, .done = true, .data = nullptr };
    auto instance = FFIData::GetData<ReadIteratorImpl>(id);
    if (!instance) {
        LOGE("[ImageSource] instance not exist %{public}" PRId64, id);
        return ret;
    }
    auto [code, done, data] = instance->Next();
    LOGI("FS_TEST::FfiOHOSFileFsReaderIteratorNext success");
    ret.code = code;
    ret.done = done;
    ret.data = data;
    return ret;
}

RetDataCArrStringN FfiOHOSFileListFile(const char* path, CListFileOptions options)
{
    LOGI("FS_TEST::FfiOHOSFileListFile start");
    auto ret = ListFileImpl::ListFile(path, options);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
    }
    LOGI("FS_TEST::FfiOHOSFileListFile end");
    return ret;
}


RetDataI64 FfiOHOSFileFsLseek(int32_t fd, int64_t offset, int whence)
{
    LOGI("FS_TEST::FfiOHOSFileFsLseek start");
    auto ret = LseekImpl::Lseek(fd, offset, whence);
    if (ret.code != SUCCESS_CODE) {
        ret.code = OHOS::CJSystemapi::FileFs::GetErrorCode(ret.code);
    }
    LOGI("FS_TEST::FfiOHOSFileFsLseek success");
    return ret;
}

int FfiOHOSFileFsFdatasync(int32_t fd)
{
    LOGI("FS_TEST::FfiOHOSFileFsFdatasync start");
    auto ret = FdatasyncImpl::Fdatasync(fd);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsFdatasync success");
    return ret;
}

int FfiOHOSFileFsFsync(int32_t fd)
{
    LOGI("FS_TEST::FfiOHOSFileFsFsync start");
    auto ret = FsyncImpl::Fsync(fd);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsFsync success");
    return ret;
}

int FfiOHOSFileFsSymlink(const char* target, const char* srcPath)
{
    LOGI("FS_TEST::FfiOHOSFileFsSymlink start");
    auto ret = SymlinkImpl::Symlink(target, srcPath);
    if (ret != SUCCESS_CODE) {
        return OHOS::CJSystemapi::FileFs::GetErrorCode(ret);
    }
    LOGI("FS_TEST::FfiOHOSFileFsSymlink success");
    return ret;
}

RetDataCString FfiOHOSFileFsReadText(char* path, int64_t offset, bool hasLen, int64_t len, char* encoding)
{
    LOGI("FS_TEST::FfiOHOSFileFsReadText start");
    RetDataCString ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = nullptr };
    auto [state, cString] = FileFsImpl::ReadText(path, offset, hasLen, len, encoding);
    LOGI("FS_TEST::FfiOHOSFileFsReadText success");
    ret.code = state;
    ret.data = cString;
    return ret;
}

int32_t FfiOHOSFileFsUtimes(char* path, double mtime)
{
    LOGI("FS_TEST::FfiOHOSFileFsUtimes start");
    auto code = FileFsImpl::Utimes(path, mtime);
    LOGI("FS_TEST::FfiOHOSFileFsUtimes success");
    return code;
}

RetDataI64 FfiOHOSFileFsCreateWatcher(char* path, uint32_t events, void (*callback)(CWatchEvent))
{
    LOGI("FS_TEST::FfiOHOSFileFsCreateWatcher start");
    RetDataI64 ret = { .code = ERR_INVALID_INSTANCE_CODE, .data = 0 };
    auto [state, nativeWatcher] = FileFsImpl::CreateWatcher(path, events, callback);
    ret.code = state;
    if (nativeWatcher == nullptr) {
        LOGI("FS_TEST::FfiOHOSFileFsCreateRandomAccessFile error");
        ret.data = 0;
        return ret;
    }
    LOGI("FS_TEST::FfiOHOSFileFsCreateWatcher success");
    ret.data = nativeWatcher->GetID();
    return ret;
}

int32_t FfiOHOSFileFsWatcherStart(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFileFsWatcherStart");
    auto instance = FFIData::GetData<WatcherImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSFileFsWatcherStart instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    LOGI("FS_TEST::FfiOHOSFileFsWatcherStart success");
    int32_t startCode = instance->StartNotify();
    if (startCode != SUCCESS_CODE) {
        return startCode;
    }
    instance->GetNotifyEvent();
    return SUCCESS_CODE;
}

int32_t FfiOHOSFileFsWatcherStop(int64_t id)
{
    LOGI("FS_TEST::FfiOHOSFileFsWatcherStart");
    auto instance = FFIData::GetData<WatcherImpl>(id);
    if (!instance) {
        LOGE("FfiOHOSFileFsWatcherStart instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    LOGI("FS_TEST::FfiOHOSFileFsWatcherStart success");
    return instance->StopNotify();
}
}
} //namespace FileFs
} //namespace CJSystemapi
} //namespace OHOS