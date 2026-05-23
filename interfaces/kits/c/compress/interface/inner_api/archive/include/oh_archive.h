/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FILE_MANAGEMENT_COMPRESS_OH_ARCHIVE_OH
#define FILE_MANAGEMENT_COMPRESS_OH_ARCHIVE_OH
#include <stdint.h>
#include "oh_archive_errcode.h"

#ifdef __cplusplus
extern "C" {

#endif

typedef struct ArchiveWriteCtx *OH_Archive_Writer_Ctx;
typedef struct ArchiveReadCtx *OH_Archive_Reader_Ctx;
typedef struct ArchiveStreamWriteCtx *OH_Archive_StreamWrite_Ctx;
typedef struct ArchiveStreamReadCtx *OH_Archive_StreamRead_Ctx;

typedef enum {
    OH_ARCHIVE_FMT_ZIP = 0,
} OH_Archive_Format;

typedef enum {
    OH_ARCHIVE_NO_COMPRESSION = 0,
    OH_ARCHIVE_COMPRESS_DEFLATE = 8,
} OH_Archive_CompressMethod;

typedef enum {
    OH_ARCHIVE_OPEN_MODE_CREATE = 0,
} OH_Archive_OpenMode;

typedef enum {
    OH_ARCHIVE_PROGRESS_CONTINUE = 0,
    OH_ARCHIVE_PROGRESS_CANCEL = 1,
} OH_Archive_ProgressType;

typedef struct {
    uint64_t totalInSize;
    uint64_t totalOutSize;
    uint32_t checksum;
} OH_Archive_StreamInfo;

typedef enum {
    OH_ARCHIVE_NO_CHECKSUM = 0,
    OH_ARCHIVE_CRC32 = 1,
} OH_Archive_StreamChecksum;

typedef struct {
    uint32_t blockSize;
    int32_t threadNum;
    OH_Archive_StreamChecksum checksum;
    OH_Archive_CompressMethod method;
} OH_Archive_Stream_Config;

typedef OH_Archive_ProgressType (*OH_Archive_ProgressHandlerWithData)(int32_t progress, void *userData);

OH_Archive_Writer_Ctx OH_Archive_Writer_OpenFile(const char *outFile,
                                                 OH_Archive_OpenMode openMode,
                                                 OH_Archive_Format fmt);

OH_Archive_ErrCode OH_Archive_Writer_SetCompressMethod(OH_Archive_Writer_Ctx arc,
                                                       OH_Archive_CompressMethod method,
                                                       int32_t compressLevel);

OH_Archive_ErrCode OH_Archive_Writer_SetProgressHandlerWithData(OH_Archive_Writer_Ctx arc,
                                                                OH_Archive_ProgressHandlerWithData progressHandler,
                                                                void *userData);

OH_Archive_ErrCode OH_Archive_Writer_Add(OH_Archive_Writer_Ctx arc,
                                         const char **infiles,
                                         uint64_t fileNum);

OH_Archive_ErrCode OH_Archive_Writer_Close(OH_Archive_Writer_Ctx arc);


typedef uint64_t (*OH_Archive_Stream_OutputHandler)(const void *data, uint64_t size, void *userData);

uint64_t OH_Archive_BufferWriteCompressBound(OH_Archive_CompressMethod method, uint64_t sourceLen);

OH_Archive_ErrCode OH_Archive_BufferWrite(uint8_t *dstBuffer, uint64_t *dstSize, const uint8_t *srcBuffer,
                                          uint64_t srcSize,
                                          OH_Archive_CompressMethod method, int32_t compressLevel);

OH_Archive_StreamWrite_Ctx OH_Archive_StreamWrite_Create(OH_Archive_Stream_Config config);

OH_Archive_ErrCode OH_Archive_StreamWrite_Start(OH_Archive_StreamWrite_Ctx ctx,
                                                OH_Archive_Stream_OutputHandler outputHandler, void *userData);

OH_Archive_ErrCode OH_Archive_StreamWrite_SetCompressLevel(OH_Archive_StreamWrite_Ctx ctx, int32_t compressLevel);

OH_Archive_ErrCode OH_Archive_StreamWrite_Cancel(OH_Archive_StreamWrite_Ctx ctx);

OH_Archive_ErrCode OH_Archive_StreamWrite_Update(OH_Archive_StreamWrite_Ctx ctx, const uint8_t *data, uint64_t size);

OH_Archive_ErrCode OH_Archive_StreamWrite_End(OH_Archive_StreamWrite_Ctx ctx, OH_Archive_StreamInfo *streamInfo);

void OH_Archive_StreamWrite_Destroy(OH_Archive_StreamWrite_Ctx ctx);

OH_Archive_Reader_Ctx OH_Archive_Reader_OpenFile(const char *inFile);

OH_Archive_ErrCode OH_Archive_Reader_SetProgressHandlerWithData(OH_Archive_Reader_Ctx arc,
                                                                OH_Archive_ProgressHandlerWithData progressHandler,
                                                                void *userData);

OH_Archive_ErrCode OH_Archive_Reader_ExtractAllFile(OH_Archive_Reader_Ctx arc, const char *outDir);

OH_Archive_ErrCode OH_Archive_Reader_Close(OH_Archive_Reader_Ctx arc);

OH_Archive_ErrCode OH_Archive_BufferRead(uint8_t *dstBuffer, uint64_t *dstSize,
                                         const uint8_t *srcBuffer, uint64_t srcSize, OH_Archive_CompressMethod method);

OH_Archive_StreamRead_Ctx OH_Archive_StreamRead_Create(OH_Archive_Stream_Config config);

OH_Archive_ErrCode OH_Archive_StreamRead_Start(OH_Archive_StreamRead_Ctx ctx,
                                               OH_Archive_Stream_OutputHandler outputHandler, void *userData);

OH_Archive_ErrCode OH_Archive_StreamRead_Cancel(OH_Archive_StreamRead_Ctx ctx);

OH_Archive_ErrCode OH_Archive_StreamRead_Update(OH_Archive_StreamRead_Ctx ctx,
                                                const uint8_t *data, uint64_t size);

OH_Archive_ErrCode OH_Archive_StreamRead_End(OH_Archive_StreamRead_Ctx ctx,
                                             OH_Archive_StreamInfo *streamInfo);

void OH_Archive_StreamRead_Destroy(OH_Archive_StreamRead_Ctx ctx);

#ifdef __cplusplus
}
#endif

#endif  // FILE_MAMAGEMENT_COMPRESS_OH_ARCHIVE_H