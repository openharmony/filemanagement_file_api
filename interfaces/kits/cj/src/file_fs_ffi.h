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

#ifndef OHOS_FILE_FS_FFI_H
#define OHOS_FILE_FS_FFI_H

#include <cstdint>

#include "cj_common_ffi.h"
#include "copy_dir.h"
#include "ffi_remote_data.h"
#include "file_fs_impl.h"
#include "list_file.h"
#include "napi/native_api.h"

extern "C" {
    FFI_EXPORT RetDataI64 FfiOHOSFileFsStatByID(int32_t file);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsStatByString(const char* file);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsCreateStream(const char* path, const char* mode);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsFdopenStream(int32_t fd, const char* mode);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsLstat(const char* path);

    // RandomAccessFile
    FFI_EXPORT napi_value FfiConvertRandomAccessFile2Napi(napi_env env, int64_t id);
    FFI_EXPORT int64_t FfiCreateRandomAccessFileFromNapi(napi_env env, napi_value objRAF);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsCreateRandomAccessFileByString(const char* file, int64_t mode);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsCreateRandomAccessFileByID(int64_t file, int64_t mode);
    FFI_EXPORT int32_t FfiOHOSRandomAccessFileGetFd(int64_t id);
    FFI_EXPORT int64_t FfiOHOSRandomAccessFileGetFPointer(int64_t id);
    FFI_EXPORT void FfiOHOSRandomAccessFileSetFilePointerSync(int64_t id, int64_t fd);
    FFI_EXPORT void FfiOHOSRandomAccessFileClose(int64_t id);
    FFI_EXPORT RetDataI64 FfiOHOSRandomAccessFileWrite(int64_t id, char* buf, size_t len, int64_t offset);
    FFI_EXPORT RetDataI64 FfiOHOSRandomAccessFileRead(int64_t id, char* buf, size_t len, int64_t offset);
    FFI_EXPORT int32_t FfiOHOSFileFsMkdir(const char* path, bool recursion, bool isTwoArgs);
    FFI_EXPORT int32_t FfiOHOSFileFsRmdir(const char* path);
    FFI_EXPORT OHOS::CJSystemapi::RetDataCArrConflictFiles FfiOHOSFileFsMoveDir(const char* src, const char* dest,
        int32_t mode);
    FFI_EXPORT int32_t FfiOHOSFileFsRename(const char* oldFile, const char* newFile);
    FFI_EXPORT int32_t FfiOHOSFileFsUnlink(const char* path);
    FFI_EXPORT RetDataBool FfiOHOSFileFsAccess(const char* path);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsOpen(const char* path, uint64_t mode);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsRead(int32_t fd, char* buffer, int64_t bufLen, size_t length, int64_t offset);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsReadCur(int32_t fd, char* buffer, int64_t bufLen, size_t length);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsWrite(int32_t fd, char* buffer, size_t length, int64_t offset,
        const char* encode);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsWriteCur(int32_t fd, char* buffer, size_t length, const char* encode);
    FFI_EXPORT OHOS::CJSystemapi::RetDataCArrConflictFiles FfiOHOSFileFsCopyDir(const char* src, const char* dest,
        int mode);
    FFI_EXPORT int FfiOHOSFileFsCopyFile(const char* src, const char* dest, int mode);
    FFI_EXPORT int FfiOHOSFileFsCopyFileSI(const char* src, int32_t dest, int mode);
    FFI_EXPORT int FfiOHOSFileFsCopyFileIS(int32_t src, const char* dest, int mode);
    FFI_EXPORT int FfiOHOSFileFsCopyFileII(int32_t src, int32_t dest, int mode);
    FFI_EXPORT int FfiOHOSFileFsMoveFile(const char* src, const char* dest, int mode);
    FFI_EXPORT RetDataCString FfiOHOSFileFsMkdtemp(const char* prefix);
    FFI_EXPORT int32_t FfiOHOSFileFsTruncateByString(const char* file, int64_t len);
    FFI_EXPORT int32_t FfiOHOSFileFsTruncateByFd(int32_t file, int64_t len);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsReadLines(char* path, char* encoding);
    FFI_EXPORT RetDataCString FfiOHOSFileFsReadText(char* path, int64_t offset, bool hasLen, int64_t len,
        char* encoding);
    FFI_EXPORT int32_t FfiOHOSFileFsUtimes(char* path, double mtime);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsCreateWatcher(char* path, uint32_t events,
        void (*callback)(OHOS::CJSystemapi::CWatchEvent));
    FFI_EXPORT int32_t FfiOHOSFileFsWatcherStart(int64_t id);
    FFI_EXPORT int32_t FfiOHOSFileFsWatcherStop(int64_t id);

    struct RetReaderIteratorResult {
        int32_t code;
        bool done;
        char* data;
    };
    FFI_EXPORT RetReaderIteratorResult FfiOHOSFileFsReaderIteratorNext(int64_t id);
    FFI_EXPORT OHOS::CJSystemapi::RetDataCArrStringN FfiOHOSFileListFile(const char* path,
        OHOS::CJSystemapi::CListFileOptions options);
    FFI_EXPORT RetDataI64 FfiOHOSFileFsLseek(int32_t fd, int64_t offset, int whence);
    FFI_EXPORT int FfiOHOSFileFsFdatasync(int32_t fd);
    FFI_EXPORT int FfiOHOSFileFsFsync(int32_t fd);
    FFI_EXPORT int FfiOHOSFileFsSymlink(const char* target, const char* srcPath);
    FFI_EXPORT int64_t FfiOHOSFileFsCreateCopyOptions(int64_t callbackId, int64_t signalId);
    FFI_EXPORT int64_t FfiOHOSFileFsCreateTaskSignal();
    FFI_EXPORT int64_t FfiOHOSFileFsTaskSignalCancel(int64_t id);
    FFI_EXPORT void FfiOHOSFileFsCopy(const char* src, const char* dest, int64_t opt);

    FFI_EXPORT int64_t FfiCreateStreamFromNapi(napi_env env, napi_value stream);
    FFI_EXPORT napi_value FfiConvertStream2Napi(napi_env env, int64_t id);
}
#endif