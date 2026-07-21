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

#include "archive_macros.h"
#include "oh_archive_reader.h"
#include "linux/limits.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdatomic.h>
#include <stdio.h>
#include "oh_archive_plugin.h"
#include "errorcode.h"

extern const FmtReaderOps g_ZipReaderFmtOps;
const FmtReaderOps *g_AllFmtReaderOps[] = {
    &g_ZipReaderFmtOps,
    NULL,
    NULL
};

static uint8_t CheckSuffix(const char *a, const char *b)
{
    while (1) {
        if (*a == '\0') {
            if (*b == '\0') {
                return 1;
            } else {
                return 0;
            }
        }

        if (tolower(*a) == tolower(*b)) {
            a++;
            b++;
        } else {
            return 0;
        }
    }
}

static ArchiveFormat GetFileArchiveFormat(const char *inFile)
{
    size_t len = strlen(inFile);
    if (len == 0) {
        return ARCHIVE_FMT_INVALID;
    }

    const char *p = inFile + len - 1;
    while (p > inFile && *p != '.') {
        p--;
    }

    if (p == inFile) {
        return ARCHIVE_FMT_INVALID;
    }

    if (CheckSuffix(p + 1, "zip")) {
        return ARCHIVE_FMT_ZIP;
    }

    return ARCHIVE_FMT_INVALID;
}

static ErrCodeMap errCodeMap[] = {
    {ARCHIVE_OK, OH_ARCHIVE_OK},
    {ARCHIVE_PARAM_ERROR, OH_ARCHIVE_PARAM_ERROR},
    {ARCHIVE_SUPPORT_ERROR, OH_ARCHIVE_UNSUPPORTED_ERROR},
    {ARCHIVE_FORMAT_ERROR, OH_ARCHIVE_UNSUPPORTED_ERROR},
    {ARCHIVE_MEM_ERROR, OH_ARCHIVE_MEM_ERROR},
    {ARCHIVE_OPEN_ERROR, OH_ARCHIVE_OPEN_ERROR},
    {ARCHIVE_WRITE_ERROR, OH_ARCHIVE_WRITE_ERROR},
    {ARCHIVE_READ_ERROR, OH_ARCHIVE_READ_ERROR},
    {ARCHIVE_NO_SPACE_ERROR, OH_ARCHIVE_NO_SPACE_ERROR},
    {ARCHIVE_EXIST_ERROR, OH_ARCHIVE_PATH_NOT_EXIST_ERROR},
    {ARCHIVE_NAME_TOO_LONG_ERROR, OH_ARCHIVE_NAME_TOO_LONG_ERROR},
    {ARCHIVE_FULL_PATH_TOO_LONG, OH_ARCHIVE_FULL_PATH_TOO_LONG_ERROR},
    {ARCHIVE_DATA_ERROR, OH_ARCHIVE_DATA_ERROR},
    {ARCHIVE_CANCEL_ERROR, OH_ARCHIVE_CANCEL_ERROR},
    {ARCHIVE_CRC_ERROR, OH_ARCHIVE_CRC_ERROR},
    {ARCHIVE_INTERNAL_ERROR, OH_ARCHIVE_UNKNOWN_ERROR},
    {ARCHIVE_EPERM_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_EACCES_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_EIO_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_ENOENT_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_END_OF_LIST, OH_ARCHIVE_OK}
};

static OH_Archive_ErrCode ConvertErrCode(int ret)
{
    int count = sizeof(errCodeMap) / sizeof(errCodeMap[0]);

    for (int i = 0; i < count; i++) {
        if (errCodeMap[i].ret == ret) {
            return errCodeMap[i].errcode;
        }
    }
    return OH_ARCHIVE_UNKNOWN_ERROR;
}

ARCHIVE_API OH_Archive_Reader_Ctx OH_Archive_Reader_OpenFile(const char *inFile)
{
    if (inFile == NULL) {
        return NULL;
    }

    ArchiveFormat fmt = GetFileArchiveFormat(inFile);
    if (fmt != ARCHIVE_FMT_ZIP) {
        return NULL;
    }

    HmArchiveReadInfo *archive = calloc(1, sizeof(HmArchiveReadInfo));
    if (archive == NULL) {
        return NULL;
    }
    archive->fmtOps = g_AllFmtReaderOps[fmt];

    // dlopen hispeed
#if defined(__aarch64__) || defined(_M_ARM64)
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL) {
        archive->fmtOps = plugin->HSDZipReaderFmtOps;
    }
#endif
    if (archive->fmtOps == NULL || archive->fmtOps->open == NULL) {
        free(archive);
        return NULL;
    }

    OH_Archive_ErrCode ret = ConvertErrCode(archive->fmtOps->open(archive, inFile));
    if (ret != OH_ARCHIVE_OK) {
        free(archive);
        return NULL;
    }
    
    return (OH_Archive_Reader_Ctx)&archive->archive;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Reader_SetProgressHandlerWithData(OH_Archive_Reader_Ctx arc,
    OH_Archive_ProgressHandlerWithData progressHandler, void *userData)
{
    if (arc == NULL || progressHandler == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    HmArchiveReadInfo *archive = (HmArchiveReadInfo *)arc;
    archive->progressHandlerWithData = progressHandler;
    archive->progressHandlerData = userData;
    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Reader_ExtractAllFile(OH_Archive_Reader_Ctx arc, const char *outDir)
{
    if (arc == NULL || outDir == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    HmArchiveReadInfo *archive = (HmArchiveReadInfo *)arc;
    if (archive->fmtOps == NULL || archive->fmtOps->extract == NULL) {
        return OH_ARCHIVE_UNKNOWN_ERROR;
    }

    size_t len = strlen(outDir);
    if (len >= PATH_MAX) {
        return OH_ARCHIVE_FULL_PATH_TOO_LONG_ERROR;
    }

    if (CreateDirectory(outDir, NULL) != ARCHIVE_OK) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    char resolvedPath[PATH_MAX];
    if (realpath(outDir, resolvedPath) == NULL) {
        perror("realpath");
        return OH_ARCHIVE_PARAM_ERROR;
    }

    return ConvertErrCode(archive->fmtOps->extract(archive, resolvedPath));
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Reader_Close(OH_Archive_Reader_Ctx arc)
{
    if (arc == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    HmArchiveReadInfo *archive = (HmArchiveReadInfo *)arc;
    if (archive->fmtOps != NULL && archive->fmtOps->close != NULL) {
        ret = ConvertErrCode(archive->fmtOps->close(archive));
    }

    if (archive != NULL) {
        free(arc);
    }

    return ret;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_BufferRead(uint8_t *dstBuffer, uint64_t *dstSize,
    const uint8_t *srcBuffer, uint64_t srcSize, OH_Archive_CompressMethod method)
{
    if (dstBuffer == NULL || dstSize == NULL || srcBuffer == NULL || srcSize == 0) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    if (method != OH_ARCHIVE_COMPRESS_DEFLATE) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    z_stream stream = {0};
    uint64_t maxDstLen = *dstSize;
    *dstSize = 0;
    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    uint64_t totalInProcessed = 0;
    uint64_t totalOutProduced = 0;
    int ret = Z_OK;
    do {
        uint64_t remainingDst = maxDstLen - totalOutProduced;
        if (remainingDst == 0) {
            ret = inflateEnd(&stream);
            return OH_ARCHIVE_INSUFFICIENT_OUTBUF_ERROR;
        }

        if (stream.avail_out == 0) {
            stream.avail_out = (uInt)(remainingDst > UINT_MAX ? UINT_MAX : remainingDst);
            stream.next_out = (Bytef *)dstBuffer + totalOutProduced;
        }

        if (stream.avail_in == 0) {
            uint64_t remainingSrc = srcSize - totalInProcessed;
            stream.avail_in = (uInt)(remainingSrc > UINT_MAX ? UINT_MAX : remainingSrc);
            stream.next_in = (z_const Bytef *)srcBuffer + totalInProcessed;
        }

        uInt availInBefore = stream.avail_in;
        uInt availOutBefore = stream.avail_out;
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            totalOutProduced += (uint64_t)(availOutBefore - stream.avail_out);
            totalInProcessed += (uint64_t)(availInBefore - stream.avail_in);
            *dstSize = totalOutProduced;
            ret = inflateEnd(&stream);
            if (ret != Z_OK) {
                return OH_ARCHIVE_DEFLATE_ERROR;
            }
            return OH_ARCHIVE_OK;
        }

        if (ret != Z_OK) {
            ret = inflateEnd(&stream);
            return OH_ARCHIVE_DEFLATE_ERROR;
        }

        totalOutProduced += (uint64_t)(availOutBefore - stream.avail_out);
        totalInProcessed += (uint64_t)(availInBefore - stream.avail_in);
    } while (totalInProcessed < srcSize);

    ret = inflateEnd(&stream);
    if (ret != Z_OK) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    *dstSize = totalOutProduced;
    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_StreamRead_Ctx OH_Archive_StreamRead_Create(OH_Archive_Stream_Config config)
{
    if (config.method != OH_ARCHIVE_COMPRESS_DEFLATE || config.blockSize == 0) {
        return NULL;
    }

    OH_Archive_StreamRead_Ctx ctx = calloc(1, sizeof(struct ArchiveStreamReadCtx));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->outBlockSize = config.blockSize;
    ctx->checksum = config.checksum;
    ctx->outBuf = calloc(ctx->outBlockSize, sizeof(uint8_t));
    if (ctx->outBuf == NULL) {
        free(ctx);
        return NULL;
    }

    if (inflateInit2(&(ctx->zlibStream), -MAX_WBITS) != Z_OK) {
        free(ctx->outBuf);
        free(ctx);
        return NULL;
    }
    return ctx;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_StreamRead_Start(OH_Archive_StreamRead_Ctx ctx,
    OH_Archive_Stream_OutputHandler outputHandler, void *userData)
{
    if (ctx == NULL || outputHandler == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    ctx->readerHandler = outputHandler;
    ctx->readHandlerData = userData;
    ctx->crc = 0;
    ctx->totalInSize = 0;
    ctx->totalOutSize = 0;
    atomic_store(&ctx->decompressCancel, 0);

    if (inflateReset2(&(ctx->zlibStream), -MAX_WBITS) != Z_OK) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_StreamRead_Cancel(OH_Archive_StreamRead_Ctx ctx)
{
    if (ctx == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    atomic_store(&ctx->decompressCancel, 1);
    return OH_ARCHIVE_OK;
}

static OH_Archive_ErrCode StreamReadHandle(OH_Archive_StreamRead_Ctx ctx)
{
    z_stream *strm = &(ctx->zlibStream);
    uint64_t outBufLen = ctx->outBlockSize - strm->avail_out;
    uint64_t handleRet = ctx->readerHandler(ctx->outBuf, outBufLen, ctx->readHandlerData);
    if (handleRet != outBufLen) {
        return OH_ARCHIVE_STREAM_OUTPUT_ERROR;
    }

    if (ctx->checksum == OH_ARCHIVE_CRC32) {
        ctx->crc = crc32(ctx->crc, ctx->outBuf, outBufLen);
    }
    ctx->totalOutSize += outBufLen;
    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_StreamRead_Update(OH_Archive_StreamRead_Ctx ctx,
    const uint8_t* data, uint64_t size)
{
    if (ctx == NULL || data == NULL || size == 0) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    z_stream *strm = &(ctx->zlibStream);
    strm->next_in = (z_const Bytef *)data;
    strm->avail_in = size;
    ctx->totalInSize += size;
    int ret = 0;

    do {
        if (atomic_load(&(ctx->decompressCancel))) {
            return OH_ARCHIVE_CANCEL_ERROR;
        }

        strm->next_out = (Bytef *)(ctx->outBuf);
        strm->avail_out = ctx->outBlockSize;
        ret = inflate(strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            OH_Archive_ErrCode handleRslt = StreamReadHandle(ctx);
            if (handleRslt != OH_ARCHIVE_OK) {
                return handleRslt;
            }
            return OH_ARCHIVE_OK;
        }

        if (ret != Z_OK) {
            return OH_ARCHIVE_DEFLATE_ERROR;
        }

        OH_Archive_ErrCode handleRslt = StreamReadHandle(ctx);
        if (handleRslt != OH_ARCHIVE_OK) {
            return handleRslt;
        }
    } while (strm->avail_in > 0 && strm->avail_out == 0);

    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_StreamRead_End(OH_Archive_StreamRead_Ctx ctx,
    OH_Archive_StreamInfo *streamInfo)
{
    if (ctx == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    if (streamInfo != NULL) {
        streamInfo->totalInSize = ctx->totalInSize;
        streamInfo->totalOutSize = ctx->totalOutSize;
        streamInfo->checksum = ctx->crc;
    }

    return OH_ARCHIVE_OK;
}

ARCHIVE_API void OH_Archive_StreamRead_Destroy(OH_Archive_StreamRead_Ctx ctx)
{
    if (ctx == NULL) {
        return;
    }

    if (ctx->outBuf != NULL) {
        free(ctx->outBuf);
    }

    (void)inflateEnd(&(ctx->zlibStream));
    free(ctx);
    return;
}

