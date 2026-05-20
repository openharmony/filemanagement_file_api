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

#ifndef HISPEED_HISPEED_ARCHIVE_PLUGIN_H
#define HISPEED_HISPEED_ARCHIVE_PLUGIN_H

#include "oh_archive.h"
#include "oh_archive_writer.h"
#include "oh_archive_reader.h"
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern FmtWriterOps g_HSDZipWriterFmtOps;
typedef uint64_t (*HSDPlugin_BufferWriteCompressBound)(OH_Archive_CompressMethod method, uint64_t sourceLen);

typedef OH_Archive_ErrCode (*HSDPlugin_BufferWrite)(uint8_t *dstBuffer, uint64_t *dstSize, const uint8_t *srcBuffer,
                                                    uint64_t srcSize,
                                                    OH_Archive_CompressMethod method, int32_t compressLevel);

typedef OH_Archive_StreamWrite_Ctx (*HSDPlugin_StreamWrite_Create)(OH_Archive_Stream_Config config);

typedef OH_Archive_ErrCode (*HSDPlugin_StreamWrite_Start)(OH_Archive_StreamWrite_Ctx ctx,
                                                          OH_Archive_Stream_OutputHandler outputHandler,
                                                          void *userData);

typedef OH_Archive_ErrCode (*HSDPlugin_StreamWrite_SetCompressLevel)(OH_Archive_StreamWrite_Ctx ctx,
                                                                     int32_t compressLevel);

typedef OH_Archive_ErrCode (*HSDPlugin_StreamWrite_Abort)(OH_Archive_StreamWrite_Ctx ctx);

typedef OH_Archive_ErrCode (*HSDPlugin_StreamWrite_Update)(OH_Archive_StreamWrite_Ctx ctx, const uint8_t *data,
                                                           uint64_t size);

typedef OH_Archive_ErrCode (*HSDPlugin_StreamWrite_End)(OH_Archive_StreamWrite_Ctx ctx,
                                                        OH_Archive_StreamInfo *streamInfo);

typedef void (*HSDPlugin_StreamWrite_Destroy)(OH_Archive_StreamWrite_Ctx ctx);

typedef struct {
    HSDPlugin_StreamWrite_Create streamWriteCreate;
    HSDPlugin_StreamWrite_Start streamWriteStart;
    HSDPlugin_StreamWrite_SetCompressLevel streamWriteSetCompressLevel;
    HSDPlugin_StreamWrite_Abort streamWriteAbort;
    HSDPlugin_StreamWrite_Update streamWriteUpdate;
    HSDPlugin_StreamWrite_End streamWriteEnd;
    HSDPlugin_StreamWrite_Destroy streamWriteDestroy;
    HSDPlugin_BufferWriteCompressBound bufferWriteCompressBound;
    HSDPlugin_BufferWrite bufferWrite;
    FmtReaderOps *HSDZipReaderFmtOps;
    FmtWriterOps zipWriterOps;
} HispeedArchivePlugin;

const HispeedArchivePlugin *GetHispeedArchivePluginHandle();

void ReleaseHispeedArchivePluginHandle();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif