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
#include <stdlib.h>
#include "stream.h"
#include "zlib.h"
#include "archive_macros.h"
#include "errorcode.h"
#include "stream_deflate.h"
#define STREAM_MAGIC_DEFLATE 0x5F570CF0

#define DEFLATE_DEF_COMPRESS_LEVEL 6
#define DEFLATE_DEF_MEM_LEVEL 8

typedef struct {
    struct Stream stream;
    z_stream zstream;
    unsigned char buf[ZIP_BUF_INTERNAL];
    int mode;
    int64_t len;
    int level;
    int method;
    int windowBits;
    int memLevel;
    int strategy;
} DeflateStream;

ARCHIVE_IMPL z_stream *DeflateGetZStream(struct Stream *stream)
{
    DeflateStream *deflateStream = (DeflateStream *)(void *)stream;
    return &deflateStream->zstream;
}

LOCAL int DeflateStreamOpen(struct Stream *stream, int mode)
{
    int ret;
    DeflateStream *deflateStream = (DeflateStream *)(void *)stream;
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_DEFLATE);
    if ((unsigned)mode & STREAM_MODE_WR) {
        deflateStream->zstream.next_in = NULL;
        deflateStream->zstream.avail_in = 0;
        ret = deflateInit2(&deflateStream->zstream, deflateStream->level, deflateStream->method,
                           deflateStream->windowBits, deflateStream->memLevel, deflateStream->strategy);
        if (ret < 0) {
            return ARCHIVE_ZLIB_ERROR;
        }
    } else {
        deflateStream->zstream.next_in = deflateStream->buf;
        deflateStream->zstream.avail_in = 0;
        ret = inflateInit2(&deflateStream->zstream, deflateStream->windowBits);
        if (ret < 0) {
            return ARCHIVE_ZLIB_ERROR;
        }
    }
    deflateStream->mode = mode;
    deflateStream->stream.totalIn = 0;
    deflateStream->stream.totalOut = 0;
    deflateStream->stream.maxTotalIn = 0;
    return ARCHIVE_OK;
}

LOCAL int64_t DeflateStreamRead(struct Stream *stream, void *buf, int64_t size)
{
    int ret;
    DeflateStream *deflateStream;
    z_stream *zstream;
    size_t len;
    uint64_t availOutBefore;
    uint64_t produced;
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_DEFLATE);
    deflateStream = (DeflateStream *)(void *)stream;
    zstream = &deflateStream->zstream;
    zstream->next_out = buf;
    zstream->avail_out = size;
    availOutBefore = zstream->avail_out;
    do {
        if (zstream->avail_in == 0) {
            len = StreamRead(stream->base, deflateStream->buf, sizeof(deflateStream->buf));
            if (len > sizeof(deflateStream->buf)) {
                return len;
            }
            zstream->next_in = deflateStream->buf;
            zstream->avail_in = len;
            deflateStream->stream.totalIn += len;
        }
        ret = inflate(zstream, Z_SYNC_FLUSH);
    } while (zstream->avail_out > 0 && ret == Z_OK);
    if (ret != Z_OK && ret != Z_STREAM_END) {
        return Z_DATA_ERROR;
    }
    produced = availOutBefore - zstream->avail_out;
    deflateStream->stream.totalOut += produced;
    return (int64_t)produced;
}

LOCAL int DeflateWriteBuf(struct Stream *stream, DeflateStream *deflateStream, z_stream *zstream, int flush)
{
    uint64_t availInBefore = zstream->avail_in;
    uint64_t availOutBefore = zstream->avail_out;
    uint64_t flushedToBase = 0;
    uint64_t processedIn;
    uint64_t processedOut;
    int ret = Z_OK;
    do {
        if (zstream->avail_out == 0) {
            int64_t len = StreamWrite(stream->base, deflateStream->buf, deflateStream->len);
            if (len != (int64_t)deflateStream->len) {
                return len < 0 ? ret : ARCHIVE_WRITE_ERROR;
            }
            flushedToBase += (uint64_t)deflateStream->len;
            zstream->next_out = deflateStream->buf;
            zstream->avail_out = deflateStream->len;
        }
        ret = deflate(zstream, flush);
        if (ret == Z_STREAM_END) {
            break;
        }
        if (ret != Z_OK) {
            (void)deflateEnd(zstream);
            return ARCHIVE_ZLIB_ERROR;
        }
    } while (zstream->avail_in > 0 || (flush == Z_FINISH && ret == Z_OK));
    processedIn = availInBefore - zstream->avail_in;
    processedOut = flushedToBase + availOutBefore - (uint64_t)zstream->avail_out;
    deflateStream->stream.totalIn += processedIn;
    deflateStream->stream.totalOut += processedOut;
    return ARCHIVE_OK;
}

LOCAL int64_t DeflateStreamWrite(struct Stream *stream, const void *buf, int64_t size)
{
    int ret;
    DeflateStream *deflateStream;
    z_stream *zstream;
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_DEFLATE);
    deflateStream = (DeflateStream *)(void *)stream;
    zstream = &deflateStream->zstream;
    zstream->next_in = (void *)buf;
    zstream->avail_in = size;
    ret = DeflateWriteBuf(stream, deflateStream, zstream, Z_NO_FLUSH);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    return size;
}

LOCAL int DeflateStreamClose(struct Stream *stream)
{
    int ret = Z_OK;
    DeflateStream *deflateStream;
    z_stream *zstream;
    int64_t writeRet;
    int64_t compressedLen;
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_DEFLATE);
    deflateStream = (DeflateStream *)(void *)stream;
    zstream = &deflateStream->zstream;
    if ((unsigned)deflateStream->mode & STREAM_MODE_WR) {
        ret = DeflateWriteBuf(stream, deflateStream, zstream, Z_FINISH);
        if (ret != ARCHIVE_OK) {
            (void)deflateEnd(zstream);
            return ret;
        }
        compressedLen = deflateStream->len - zstream->avail_out;
        writeRet = StreamWrite(stream->base, deflateStream->buf, compressedLen);
        if (writeRet != compressedLen) {
            (void)deflateEnd(zstream);
            return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
        }
        (void)deflateEnd(zstream);
    } else {
        ret = inflateEnd(zstream);
        RETURN_IF_FAIL(ret);
    }
    return ARCHIVE_OK;
}

LOCAL const StreamOps g_deflateStreamOps = {
    .open = DeflateStreamOpen,
    .read = DeflateStreamRead,
    .write = DeflateStreamWrite,
    .close = DeflateStreamClose,
    .seek = NULL,
    .tell = NULL,
};

ARCHIVE_IMPL struct Stream *DeflateStreamCreateSpec(int method, int level, int windowBits, int memLevel, int strategy)
{
    DeflateStream *deflateStream = malloc(sizeof(DeflateStream));
    if (deflateStream == NULL) {
        return NULL;
    }
    SET_OBJ_MAGIC(&deflateStream->stream, STREAM_MAGIC_DEFLATE);
    deflateStream->stream.ops = g_deflateStreamOps;
    deflateStream->level = level;
    deflateStream->method = method;
    deflateStream->windowBits = windowBits;
    deflateStream->memLevel = memLevel;
    deflateStream->strategy = strategy;
    deflateStream->len = sizeof(deflateStream->buf);
    deflateStream->zstream.next_in = NULL;
    deflateStream->zstream.avail_in = 0;
    deflateStream->zstream.next_out = deflateStream->buf;
    deflateStream->zstream.avail_out = sizeof(deflateStream->buf);
    deflateStream->zstream.zalloc = Z_NULL;
    deflateStream->zstream.zfree = Z_NULL;
    deflateStream->zstream.opaque = Z_NULL;
    return (struct Stream *)deflateStream;
}

ARCHIVE_IMPL struct Stream *DeflateStreamCreate(void)
{
    return DeflateStreamCreateSpec(Z_DEFLATED, DEFLATE_DEF_COMPRESS_LEVEL,
                                   -MAX_WBITS, DEFLATE_DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
}
