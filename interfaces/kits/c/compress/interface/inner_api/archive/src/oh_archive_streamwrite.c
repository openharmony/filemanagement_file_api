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

#include <errno.h>
#include <securec.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>

#include "archive_macros.h"
#include "oh_archive.h"
#include "oh_archive_plugin.h"
#include "zlib.h"

#define DEFLATE_DEFAULT_MEM_LEVEL (8)
#define DEFLATE_DEFAULT_LEVEL (6)
#define DEFLATE_MIN_LEVEL (-1)
#define DEFLATE_MAX_LEVEL (9)

#define MEM_BLOCK_NUM(p) ((((unsigned int)(p)) << 1))
#define OUT_BUF_SIZE(s) ((s) + ((s) >> 4))

#define EXPORT_API __attribute__((visibility("default")))

#define DEFLATE_BLOCK_SIZE_THRESHOLD (32 * 1024)
#define DEFLATE_BUFFER_BLOCK_SIZE (64 * 1024)

#if defined(__aarch64__) || defined(_M_ARM64)
#ifndef LOAD_ARCHIVE_PLUGIN
#define LOAD_ARCHIVE_PLUGIN
#endif
#endif

typedef OH_Archive_ErrCode (*PushDataRoutine)(OH_Archive_StreamWrite_Ctx ctx, const uint8_t *data, uint64_t size);

typedef enum {
    W_STREAM_CREATED = 0,
    W_STREAM_STARTED,
    W_STREAM_CANCELED,
    W_STREAM_ENDED,
} WriteStreamState;

struct ArchiveStreamWriteCtx {
    OH_Archive_Stream_Config config;
    OH_Archive_Stream_OutputHandler writeCallback;
    void *userData;
    int level;
    _Atomic int result;
    uint64_t totalIn;
    uint64_t totalOut;
    uint32_t crc32;

    PushDataRoutine pushDataRoutine;

    z_stream zstream;
    _Atomic WriteStreamState state;
    uint8_t *buf;
    uint64_t bufSize;
};

static int CheckStreamWriteCreateParam(OH_Archive_Stream_Config *config)
{
    if (config->method != OH_ARCHIVE_COMPRESS_DEFLATE) {
        return -1;
    }
    if (config->method == OH_ARCHIVE_COMPRESS_DEFLATE && config->blockSize < DEFLATE_BLOCK_SIZE_THRESHOLD) {
        return -1;
    }
    if (config->checksum < 0 || config->checksum > OH_ARCHIVE_CRC32) {
        return -1;
    }

    return 0;
}

static OH_Archive_ErrCode FlushInternalData(OH_Archive_StreamWrite_Ctx ctx, int flushMode)
{
    int ret = Z_OK;
    do {
        if (atomic_load(&ctx->result) != OH_ARCHIVE_OK) {
            return ctx->result;
        }
        if (ctx->zstream.avail_out == 0) {
            uint64_t len = ctx->writeCallback(ctx->buf, ctx->bufSize, ctx->userData);
            if (len != ctx->bufSize) {
                return OH_ARCHIVE_STREAM_OUTPUT_ERROR;
            }
            ctx->zstream.next_out = ctx->buf;
            ctx->zstream.avail_out = ctx->bufSize;
            ctx->totalOut += ctx->bufSize;
        }

        ret = deflate(&ctx->zstream, flushMode);
        if (ret == Z_STREAM_END) {
            break;
        }
        if (ret != Z_OK) {
            (void)deflateEnd(&ctx->zstream);
            return OH_ARCHIVE_DATA_ERROR;
        }
    } while (ctx->zstream.avail_in > 0 || (flushMode == Z_FINISH && ret == Z_OK));
    return OH_ARCHIVE_OK;
}

static OH_Archive_ErrCode PushDataSequential(OH_Archive_StreamWrite_Ctx ctx, const uint8_t *data, uint64_t size)
{
    z_stream *strm = &ctx->zstream;
    strm->next_in = (void *)data;
    strm->avail_in = size;
    ctx->totalIn += size;
    if (ctx->config.checksum == OH_ARCHIVE_CRC32) {
        ctx->crc32 = crc32(ctx->crc32, data, size);
    }

    return FlushInternalData(ctx, Z_NO_FLUSH);
}

static void InitStreamWriteCtx(OH_Archive_StreamWrite_Ctx ctx, OH_Archive_Stream_Config *config)
{
    ctx->result = OH_ARCHIVE_OK;
    ctx->config = *config;
    ctx->state = W_STREAM_CREATED;
    ctx->bufSize = config->blockSize;
    ctx->pushDataRoutine = PushDataSequential;
}

EXPORT_API OH_Archive_StreamWrite_Ctx OH_Archive_StreamWrite_Create(OH_Archive_Stream_Config config)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteCreate != NULL) {
        return plugin->streamWriteCreate(config);
    }

    if (CheckStreamWriteCreateParam(&config) != 0) {
        return NULL;
    }

    OH_Archive_StreamWrite_Ctx ctx = (OH_Archive_StreamWrite_Ctx)malloc(sizeof(struct ArchiveStreamWriteCtx));
    if (ctx == NULL) {
        ARCHIVE_ERR("OH_Archive_StreamWrite_Ctx create failed\n");
        return NULL;
    }

    ctx->buf = (uint8_t *)malloc(config.blockSize);
    if (ctx->buf == NULL) {
        ARCHIVE_ERR("Inner buffer create failed\n");
        free(ctx);
        return NULL;
    }

    ctx->level = DEFLATE_DEFAULT_LEVEL;
    memset_s(&ctx->zstream, sizeof(z_stream), 0, sizeof(z_stream));
    if (Z_OK != deflateInit2(&ctx->zstream, ctx->level, Z_DEFLATED, -MAX_WBITS, DEFLATE_DEFAULT_MEM_LEVEL,
                             Z_DEFAULT_STRATEGY)) {
        free(ctx->buf);
        free(ctx);
        return NULL;
    }
    InitStreamWriteCtx(ctx, &config);
    return ctx;
}

EXPORT_API OH_Archive_ErrCode OH_Archive_StreamWrite_SetCompressLevel(OH_Archive_StreamWrite_Ctx ctx, int compressLevel)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteSetCompressLevel != NULL) {
        return plugin->streamWriteSetCompressLevel(ctx, compressLevel);
    }

    if (ctx == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (compressLevel < DEFLATE_MIN_LEVEL || compressLevel > DEFLATE_MAX_LEVEL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (compressLevel == DEFLATE_MIN_LEVEL) {
        compressLevel = DEFLATE_DEFAULT_LEVEL;
    }
    if (ctx->state != W_STREAM_CREATED && ctx->state != W_STREAM_ENDED) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (ctx->level == compressLevel) {
        return OH_ARCHIVE_OK;
    }
    if (deflateParams(&ctx->zstream, compressLevel, Z_DEFAULT_STRATEGY) != Z_OK) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    ctx->level = compressLevel;
    ctx->pushDataRoutine = PushDataSequential;

    return OH_ARCHIVE_OK;
}

EXPORT_API OH_Archive_ErrCode OH_Archive_StreamWrite_Abort(OH_Archive_StreamWrite_Ctx ctx)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteAbort != NULL) {
        return plugin->streamWriteAbort(ctx);
    }
    if (ctx == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (ctx->state != W_STREAM_STARTED) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    atomic_store(&ctx->result, OH_ARCHIVE_CANCEL_ERROR);
    deflateReset(&ctx->zstream);
    atomic_store(&ctx->state, W_STREAM_CANCELED);
    return OH_ARCHIVE_OK;
}

EXPORT_API OH_Archive_ErrCode OH_Archive_StreamWrite_Start(OH_Archive_StreamWrite_Ctx ctx,
                                                           OH_Archive_Stream_OutputHandler writeCallback,
                                                           void *userData)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteStart != NULL) {
        return plugin->streamWriteStart(ctx, writeCallback, userData);
    }
    if (ctx == NULL || writeCallback == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    if (ctx->state != W_STREAM_CREATED && ctx->state != W_STREAM_ENDED && ctx->state != W_STREAM_CANCELED) {
        OH_Archive_StreamWrite_Abort(ctx);
    }
    if (deflateReset(&ctx->zstream) != Z_OK) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }
    ctx->zstream.next_out = ctx->buf;
    ctx->zstream.avail_out = ctx->bufSize;

    atomic_store(&ctx->state, W_STREAM_STARTED);
    ctx->result = OH_ARCHIVE_OK;
    ctx->totalOut = 0;
    ctx->totalIn = 0;
    ctx->crc32 = 0;
    ctx->writeCallback = writeCallback;
    ctx->userData = userData;

    return OH_ARCHIVE_OK;
}

EXPORT_API OH_Archive_ErrCode OH_Archive_StreamWrite_Update(OH_Archive_StreamWrite_Ctx ctx, const uint8_t *data,
                                                            uint64_t size)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteUpdate != NULL) {
        return plugin->streamWriteUpdate(ctx, data, size);
    }
    if (ctx == NULL || data == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (atomic_load(&ctx->result) != OH_ARCHIVE_OK) {
        return ctx->result;
    }
    if (ctx->state != W_STREAM_STARTED) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    return ctx->pushDataRoutine(ctx, data, size);
}

EXPORT_API OH_Archive_ErrCode OH_Archive_StreamWrite_End(OH_Archive_StreamWrite_Ctx ctx,
                                                         OH_Archive_StreamInfo *compressInfo)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteEnd != NULL) {
        return plugin->streamWriteEnd(ctx, compressInfo);
    }
    if (ctx == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (ctx->state != W_STREAM_STARTED && ctx->state != W_STREAM_CANCELED) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    int result = FlushInternalData(ctx, Z_FINISH);
    if (result != OH_ARCHIVE_OK) {
        return result;
    }
    uint64_t remainedSize = ctx->bufSize - ctx->zstream.avail_out;
    uint64_t len = ctx->writeCallback(ctx->buf, remainedSize, ctx->userData);
    if (len != remainedSize) {
        return OH_ARCHIVE_STREAM_OUTPUT_ERROR;
    }
    ctx->totalOut += remainedSize;

    if (compressInfo != NULL) {
        compressInfo->totalInSize = ctx->totalIn;
        compressInfo->totalOutSize = ctx->totalOut;
        compressInfo->checksum = ctx->crc32;
    }
    atomic_store(&ctx->state, W_STREAM_ENDED);
    return result;
}

EXPORT_API void OH_Archive_StreamWrite_Destroy(OH_Archive_StreamWrite_Ctx ctx)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->streamWriteDestroy != NULL) {
        plugin->streamWriteDestroy(ctx);
        return;
    }
    if (ctx == NULL) {
        return;
    }
    free(ctx->buf);
    ctx->buf = NULL;
    deflateEnd(&ctx->zstream);
    free(ctx);
}

struct SourceMgr {
    uint8_t *buf;
    uint64_t size;
};

static void StreamNext(z_stream *stream, uLong *left, uLong *sourceLen)
{
    const uInt max = (uInt)-1;
    if (stream->avail_out == 0) {
        stream->avail_out = *left > (uLong)max ? max : (uInt)*left;
        *left -= stream->avail_out;
    }
    if (stream->avail_in == 0) {
        stream->avail_in = *sourceLen > (uLong)max ? max : (uInt)*sourceLen;
        *sourceLen -= stream->avail_in;
    }
}

static int CompressDeflate(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level)
{
    z_stream stream = {0};
    uLong left;
    int err;

    if (destLen == NULL) {
        return Z_ERRNO;
    }

    left = *destLen;
    *destLen = 0;

    err = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, DEFLATE_DEFAULT_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (err != Z_OK) {
        return err;
    }

    stream.next_out = dest;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_out = 0;
    stream.avail_in = 0;

    do {
        StreamNext(&stream, &left, &sourceLen);
        err = deflate(&stream, (sourceLen != 0) ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);

    *destLen = stream.total_out;
    deflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}

EXPORT_API OH_Archive_ErrCode OH_Archive_BufferWrite(uint8_t *dest, uint64_t *destLen, const uint8_t *source,
                                                     uint64_t sourceLen, OH_Archive_CompressMethod method,
                                                     int32_t level)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->bufferWrite != NULL) {
        return plugin->bufferWrite(dest, destLen, source, sourceLen, method, level);
    }

    if (source == NULL || dest == NULL || destLen == NULL || sourceLen == 0 || method != OH_ARCHIVE_COMPRESS_DEFLATE) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    int ret = CompressDeflate(dest, (uLongf *)destLen, source, sourceLen, level);
    if (ret == Z_BUF_ERROR) {
        return OH_ARCHIVE_INSUFFICIENT_OUTBUF_ERROR;
    } else if (ret == Z_MEM_ERROR) {
        return OH_ARCHIVE_MEM_ERROR;
    } else if (ret == Z_STREAM_ERROR) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    return OH_ARCHIVE_OK;
}

EXPORT_API uint64_t OH_Archive_BufferWriteCompressBound(OH_Archive_CompressMethod method, uint64_t sourceLen)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin != NULL && plugin->bufferWriteCompressBound != NULL) {
        return plugin->bufferWriteCompressBound(method, sourceLen);
    }

    if (method == OH_ARCHIVE_COMPRESS_DEFLATE) {
        uint64_t blockBound = compressBound(DEFLATE_BUFFER_BLOCK_SIZE);
        uint64_t blockCount = (sourceLen + DEFLATE_BUFFER_BLOCK_SIZE - 1) / DEFLATE_BUFFER_BLOCK_SIZE;
        return blockBound * blockCount;
    }
    return 0;
}