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
#include "stream.h"
#include "archive_macros.h"
#include "errorcode.h"

ARCHIVE_IMPL int StreamOpen(struct Stream *stream, int mode)
{
    ASSERT(stream != NULL);
    return stream->ops.open(stream, mode);
}

ARCHIVE_IMPL int64_t StreamRead(struct Stream *stream, void *buf, int64_t size)
{
    ASSERT(stream != NULL);
    ASSERT(buf != NULL);
    return stream->ops.read(stream, buf, size);
}

ARCHIVE_IMPL int64_t StreamWrite(struct Stream *stream, const void *buf, int64_t size)
{
    ASSERT(stream != NULL);
    if (buf == NULL) {
        return 0;
    }
    return stream->ops.write(stream, (void *)buf, size);
}

ARCHIVE_IMPL int StreamSeek(struct Stream *stream, int64_t offset, int origin)
{
    ASSERT(stream != NULL);
    return stream->ops.seek(stream, offset, origin);
}

ARCHIVE_IMPL int64_t StreamTell(struct Stream *stream)
{
    ASSERT(stream != NULL);
    return stream->ops.tell(stream);
}

ARCHIVE_IMPL int StreamClose(struct Stream *stream)
{
    ASSERT(stream != NULL);
    return stream->ops.close(stream);
}

ARCHIVE_IMPL void StreamDestroy(struct Stream **stream)
{
    ASSERT(stream != NULL);
    ASSERT(*stream != NULL);

    free(*stream);
    *stream = NULL;
}

ARCHIVE_IMPL void StreamSetBase(struct Stream *stream, struct Stream *base)
{
    ASSERT(stream != NULL);
    ASSERT(base != NULL);
    stream->base = base;
}

ARCHIVE_IMPL uint64_t StreamGetTotalIn(struct Stream *stream)
{
    ASSERT(stream != NULL);
    return stream->totalIn;
}

ARCHIVE_IMPL uint64_t StreamGetTotalOut(struct Stream *stream)
{
    ASSERT(stream != NULL);
    return stream->totalOut;
}

ARCHIVE_IMPL uint32_t StreamGetHeaderSize(struct Stream *stream)
{
    ASSERT(stream != NULL);
    return stream->headerSize;
}

ARCHIVE_IMPL void StreamSetMaxTotalIn(struct Stream *stream, uint64_t maxTotalIn)
{
    ASSERT(stream != NULL);
    stream->maxTotalIn = maxTotalIn;
}

#define BYTE_BITS_NUM 8

static int32_t StreamWriteValue(void *stream, uint64_t value, uint8_t len)
{
    uint8_t buf[8];
    size_t n = 0;
    for (n = 0; n < len; n += 1) {
        buf[n] = (uint8_t)(value & 0xff);
        value >>= BYTE_BITS_NUM;
    }

    if (value != 0) {
        for (n = 0; n < len; n += 1) {
            buf[n] = 0xff;
        }
    }

    if (StreamWrite(stream, buf, len) != len)
        return errno == ENOSPC ? ARCHIVE_NO_SPACE_ERROR : ARCHIVE_WRITE_ERROR;

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int StreamWriteUint8(struct Stream *stream, uint8_t value)
{
    return StreamWriteValue(stream, value, sizeof(uint8_t));
}

ARCHIVE_IMPL int StreamWriteUint16(struct Stream *stream, uint16_t value)
{
    return StreamWriteValue(stream, value, sizeof(uint16_t));
}

ARCHIVE_IMPL int StreamWriteUint32(struct Stream *stream, uint32_t value)
{
    return StreamWriteValue(stream, value, sizeof(uint32_t));
}

ARCHIVE_IMPL int StreamWriteUint64(struct Stream *stream, uint64_t value)
{
    return StreamWriteValue(stream, value, sizeof(uint64_t));
}

ARCHIVE_IMPL int StreamCopyStream(struct Stream *targetStream, struct Stream *sourceStream, uint64_t size)
{
    ASSERT(targetStream != NULL);
    ASSERT(sourceStream != NULL);
    uint8_t buf[16384];
    size_t copybytes = 0;
    size_t read = 0;
    size_t written = 0;

    while (size > 0) {
        copybytes = size;
        if (copybytes > sizeof(buf)) {
            copybytes = sizeof(buf);
        }
        read = StreamRead(sourceStream, buf, copybytes);
        if (read != copybytes) {
            return ARCHIVE_MEM_ERROR;
        }
        written = StreamWrite(targetStream, buf, read);
        if (written != read) {
            return ARCHIVE_MEM_ERROR;
        }
        size -= read;
    }
    return ARCHIVE_OK;
}