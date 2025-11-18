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

#include "cj_common_ffi.h"

extern "C" {
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
FFI_EXPORT int FfiOHOSFileFsReleaseCString = 0;
FFI_EXPORT int FfiOHOSFileFsClose = 0;
FFI_EXPORT int FfiOHOSFileFsCloseByFd = 0;
FFI_EXPORT int FfiOHOSFILEFsGetFD = 0;
FFI_EXPORT int FfiOHOSFileFsOpen = 0;
FFI_EXPORT int FfiOHOSFileFsStatByID = 0;
FFI_EXPORT int FfiOHOSFileFsStatByString = 0;
FFI_EXPORT int FfiOHOSFileFsMkdir = 0;
FFI_EXPORT int FfiOHOSFileFsRmdir = 0;
FFI_EXPORT int FfiOHOSFileFsRename = 0;
FFI_EXPORT int FfiOHOSFileFsUnlink = 0;
FFI_EXPORT int FfiOHOSFileFsRead = 0;
FFI_EXPORT int FfiOHOSFileFsReadCur = 0;
FFI_EXPORT int FfiOHOSFileFsWrite = 0;
FFI_EXPORT int FfiOHOSFileFsWriteCur = 0;
FFI_EXPORT int FfiOHOSFileFsMkdtemp = 0;
FFI_EXPORT int FfiOHOSFileFsTruncateByString = 0;
FFI_EXPORT int FfiOHOSFileFsTruncateByFd = 0;
FFI_EXPORT int FfiOHOSFileFsFdatasync = 0;
FFI_EXPORT int FfiOHOSFileFsFsync = 0;
FFI_EXPORT int FfiOHOSStatGetIno = 0;
FFI_EXPORT int FfiOHOSStatGetMode = 0;
FFI_EXPORT int FfiOHOSStatGetUid = 0;
FFI_EXPORT int FfiOHOSStatGetGid = 0;
FFI_EXPORT int FfiOHOSStatGetSize = 0;
FFI_EXPORT int FfiOHOSStatGetAtime = 0;
FFI_EXPORT int FfiOHOSStatGetMtime = 0;
FFI_EXPORT int FfiOHOSStatGetCtime = 0;
FFI_EXPORT int FfiOHOSStatIsBlockDevice = 0;
FFI_EXPORT int FfiOHOSStatIsCharacterDevice = 0;
FFI_EXPORT int FfiOHOSStatIsDirectory = 0;
FFI_EXPORT int FfiOHOSStatIsFIFO = 0;
FFI_EXPORT int FfiOHOSStatIsFile = 0;
FFI_EXPORT int FfiOHOSStatIsSocket = 0;
FFI_EXPORT int FfiOHOSStatIsSymbolicLink = 0;
FFI_EXPORT int FfiOHOSStatIsBlockDeviceV2 = 0;
FFI_EXPORT int FfiOHOSStatIsCharacterDeviceV2 = 0;
FFI_EXPORT int FfiOHOSStatIsDirectoryV2 = 0;
FFI_EXPORT int FfiOHOSStatIsFIFOV2 = 0;
FFI_EXPORT int FfiOHOSStatIsFileV2 = 0;
FFI_EXPORT int FfiOHOSStatIsSocketV2 = 0;
FFI_EXPORT int FfiOHOSStatIsSymbolicLinkV2 = 0;
#endif
FFI_EXPORT int FfiOHOSFileFsDup = 0;
FFI_EXPORT int FfiOHOSFILEFsGetPath = 0;
FFI_EXPORT int FfiOHOSFILEFsGetName = 0;
FFI_EXPORT int FfiOHOSFILEFsTryLock = 0;
FFI_EXPORT int FfiOHOSFILEFsUnLock = 0;
FFI_EXPORT int FfiOHOSFILEFsGetParent = 0;
FFI_EXPORT int FfiOHOSFileFsCreateStream = 0;
FFI_EXPORT int FfiOHOSFileFsFdopenStream = 0;
FFI_EXPORT int FfiOHOSFileFsLstat = 0;
FFI_EXPORT int FfiOHOSFileFsCreateRandomAccessFileByString = 0;
FFI_EXPORT int FfiOHOSFileFsCreateRandomAccessFileByID = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileGetFd = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileGetFPointer = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileSetFilePointerSync = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileClose = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileWrite = 0;
FFI_EXPORT int FfiOHOSRandomAccessFileRead = 0;
FFI_EXPORT int FfiOHOSFileFsMoveDir = 0;
FFI_EXPORT int FfiOHOSFileFsAccess = 0;
FFI_EXPORT int FfiOHOSFileFsCopyDir = 0;
FFI_EXPORT int FfiOHOSFileFsCopyFile = 0;
FFI_EXPORT int FfiOHOSFileFsCopyFileSI = 0;
FFI_EXPORT int FfiOHOSFileFsCopyFileIS = 0;
FFI_EXPORT int FfiOHOSFileFsCopyFileII = 0;
FFI_EXPORT int FfiOHOSFileFsMoveFile = 0;
FFI_EXPORT int FfiOHOSFileFsReadLines = 0;
FFI_EXPORT int FfiOHOSFileFsReadText = 0;
FFI_EXPORT int FfiOHOSFileFsUtimes = 0;
FFI_EXPORT int FfiOHOSFileFsCreateWatcher = 0;
FFI_EXPORT int FfiOHOSFileFsWatcherStart = 0;
FFI_EXPORT int FfiOHOSFileFsWatcherStop = 0;
FFI_EXPORT int FfiOHOSFileFsReaderIteratorNext = 0;
FFI_EXPORT int FfiOHOSFileListFile = 0;
FFI_EXPORT int FfiOHOSFileFsLseek = 0;
FFI_EXPORT int FfiOHOSFileFsSymlink = 0;
FFI_EXPORT int FfiOHOSStreamClose = 0;
FFI_EXPORT int FfiOHOSStreamFlush = 0;
FFI_EXPORT int FfiOHOSStreamWriteCur = 0;
FFI_EXPORT int FfiOHOSStreamWrite = 0;
FFI_EXPORT int FfiOHOSStreamReadCur = 0;
FFI_EXPORT int FfiOHOSStreamRead = 0;
FFI_EXPORT int FfiOHOSFileFsCreateCopyOptions = 0;
FFI_EXPORT int FfiOHOSFileFsCreateTaskSignal = 0;
FFI_EXPORT int FfiOHOSFileFsTaskSignalCancel = 0;
FFI_EXPORT int FfiOHOSFileFsCopy = 0;
FFI_EXPORT int FfiCreateFileFromNapi = 0;
FFI_EXPORT int FfiConvertFile2Napi = 0;
FFI_EXPORT int FfiCreateRandomAccessFileFromNapi = 0;
FFI_EXPORT int FfiConvertRandomAccessFile2Napi = 0;
FFI_EXPORT int FfiCreateStreamFromNapi = 0;
FFI_EXPORT int FfiConvertStream2Napi = 0;
FFI_EXPORT int FfiOHOSStatGetLocation = 0;
FFI_EXPORT int FfiOHOSFileFsAccessExt = 0;
FFI_EXPORT int FfiOHOSFileFsSetXattr = 0;
FFI_EXPORT int FfiOHOSFileFsGetXattr = 0;
}
