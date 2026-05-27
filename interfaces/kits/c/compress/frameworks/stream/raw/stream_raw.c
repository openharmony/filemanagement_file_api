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

#include "stream_raw.h"
#include <stdlib.h>
#include "stream.h"
#include "archive_macros.h"
#include "errorcode.h"
#define STREAM_MAGIC_RAW 0x2A5ED56F

typedef struct {
    struct Stream stream;
} RawStream;

LOCAL int RawStreamOpen(struct Stream *stream, int mode)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_RAW);
    ARCHIVE_UNUSED(mode);
    RawStream *raw = (RawStream *)stream;
    raw->stream.totalIn = 0;
    raw->stream.totalOut = 0;
    raw->stream.maxTotalIn = 0;
    raw->stream.headerSize = 0;
    return ARCHIVE_OK;
}

LOCAL int64_t RawStreamRead(struct Stream *stream, void *buf, int64_t len)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_RAW);
    RawStream *raw = (RawStream *)stream;
    int64_t readSize = len;
    int64_t read;
    if (raw->stream.maxTotalIn > 0 && (uint64_t)readSize > raw->stream.maxTotalIn - raw->stream.totalIn) {
        readSize = raw->stream.maxTotalIn - raw->stream.totalIn;
    }
    read = StreamRead(raw->stream.base, buf, readSize);
    if (read > 0) {
        raw->stream.totalIn += read;
        raw->stream.totalOut += read;
    }
    return read;
}

LOCAL int64_t RawStreamWrite(struct Stream *stream, const void *buf, int64_t len)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_RAW);

    RawStream *raw = (RawStream *)stream;
    int64_t writeSize = StreamWrite(raw->stream.base, buf, len);
    if (writeSize == len) {
        raw->stream.totalIn += len;
        raw->stream.totalOut += len;
    }
    return writeSize;
}

LOCAL int RawStreamClose(struct Stream *stream)
{
    ASSERT(stream);
    ARCHIVE_UNUSED(stream);
    return ARCHIVE_OK;
}

LOCAL int RawStreamSeek(struct Stream *stream, int64_t offset, int origin)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_RAW);

    RawStream *raw = (RawStream *)stream;
    return StreamSeek(raw->stream.base, offset, origin);
}

LOCAL int64_t RawStreamTell(struct Stream *stream)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_RAW);

    RawStream *raw = (RawStream *)stream;
    return StreamTell(raw->stream.base);
}

LOCAL const StreamOps g_RawStreamOps = {
    .open = RawStreamOpen,
    .read = RawStreamRead,
    .write = RawStreamWrite,
    .seek = RawStreamSeek,
    .tell = RawStreamTell,
    .close = RawStreamClose,
};

ARCHIVE_IMPL struct Stream *RawStreamCreate(void)
{
    RawStream *raw = calloc(1, sizeof(RawStream));
    if (raw == NULL) {
        ARCHIVE_ERR("RawStreamCreate fail!\n");
        return NULL;
    }
    SET_OBJ_MAGIC(&raw->stream, STREAM_MAGIC_RAW);
    raw->stream.ops = g_RawStreamOps;
    return (struct Stream *)raw;
}