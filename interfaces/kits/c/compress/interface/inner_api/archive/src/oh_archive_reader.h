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

#ifndef OH_ARCHIVE_READER_H
#define OH_ARCHIVE_READER_H

#include "archive_inner.h"
#include "oh_archive.h"
#include "zlib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HmArchiveReadInfoS;
typedef struct HmArchiveReadInfoS HmArchiveReadInfo;

typedef int (*FmtReaderOpenFunc)(HmArchiveReadInfo *archive, const char *inFile);
typedef int (*FmtReaderExtractFunc)(HmArchiveReadInfo *archive, const char *outDir);
typedef int (*FmtReaderCloseFunc)(HmArchiveReadInfo *archive);

typedef struct {
    FmtReaderOpenFunc open;
    FmtReaderExtractFunc extract;
    FmtReaderCloseFunc close;
} FmtReaderOps;

struct HmArchiveReadInfoS {
    struct HmArchiveS archive;
    OH_Archive_ProgressHandlerWithData progressHandlerWithData;
    void *progressHandlerData;
    const FmtReaderOps *fmtOps;
    void *fmtInfo;
};

struct ArchiveStreamReadCtx {
    z_stream zlibStream;
    uint32_t outBlockSize;
    uint8_t *outBuf;
    OH_Archive_StreamChecksumAlg checksum;
    OH_Archive_Stream_OutputHandler readerHandler;
    void *readHandlerData;
    _Atomic uint8_t decompressCancel;
    uint32_t crc;
    uint64_t totalInSize;
    uint64_t totalOutSize;
};

#ifdef __cplusplus
}
#endif

#endif /* OH_ARCHIVE_READER_H */