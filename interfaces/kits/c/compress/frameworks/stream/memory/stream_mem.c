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
#include <string.h>
#include <securec.h>
#include "stream.h"
#include "stream_mem.h"

#include "archive_macros.h"
#include "errorcode.h"
#define STREAM_MAGIC_MEM 0x1C9B2FF4
#define MEM_INIT_SIZE (4096)

typedef struct {
    struct Stream stream;
    uint8_t *buffer;
    size_t size;
    size_t capacity;
    size_t position;
    size_t memInitSize;
} MemStream;

LOCAL int safeMemoryCopy(uint8_t *newBuf, size_t destLen, const uint8_t *buf, size_t len)
{
    int ret;
    size_t cnt = len / SECUREC_MEM_MAX_LEN;
    size_t remainder = len % SECUREC_MEM_MAX_LEN;
    for (size_t i = 0; i < cnt; i++) {
        ret = memcpy_s(newBuf, SECUREC_MEM_MAX_LEN, buf, SECUREC_MEM_MAX_LEN);
        if (ret != EOK) {
            return ARCHIVE_INTERNAL_ERROR;
        }
        newBuf += SECUREC_MEM_MAX_LEN;
        buf += SECUREC_MEM_MAX_LEN;
    }
    if (remainder > 0) {
        ret = memcpy_s(newBuf, remainder, buf, remainder);
        if (ret != EOK) {
            return ARCHIVE_INTERNAL_ERROR;
        }
    }
    return ARCHIVE_OK;
}

LOCAL int MemStreamResize(MemStream *mem, size_t size)
{
    int ret;
    uint8_t *newBuf;
    size_t copySize = 0;
    if (size == 0) {
        return ARCHIVE_PARAM_ERROR;
    }
    newBuf = (uint8_t *)malloc(size);
    if (newBuf == NULL) {
        return ARCHIVE_MEM_ERROR;
    }

    if (mem->buffer) {
        copySize = mem->size < size ? mem->size : size;
        ret = safeMemoryCopy(newBuf, size, mem->buffer, copySize);
        if (ret != ARCHIVE_OK) {
            free(newBuf);
            return ret;
        }
        free(mem->buffer);
    }
    mem->buffer = newBuf;
    mem->size = copySize;
    mem->capacity = size;

    return ARCHIVE_OK;
}

LOCAL int MemStreamOpen(struct Stream *stream, int mode)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    ARCHIVE_UNUSED(mode);
    MemStream *mem = (MemStream *)stream;
    mem->position = 0;

    return MemStreamResize(mem, mem->memInitSize);
}

LOCAL int64_t MemStreamRead(struct Stream *stream, void *buf, int64_t len)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);
    MemStream *mem = (MemStream *)stream;
    ASSERT((mem->size >= mem->position));
    if ((uint64_t)len > mem->size - mem->position) {
        len = mem->size - mem->position;
    }

    if (len == 0) {
        return 0;
    }
    int ret = safeMemoryCopy(buf, len, mem->buffer + mem->position, len);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    mem->position += len;

    return len;
}

LOCAL int64_t MemStreamWrite(struct Stream *stream, const void *buf, int64_t len)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;
    int ret;
    if (len == 0) {
        return 0;
    }

    if ((size_t)len > mem->capacity - mem->position) {
        size_t newCap = mem->capacity << 1;
        while ((size_t)len > newCap - mem->position) {
            newCap <<= 1;
        }
        ret = MemStreamResize(mem, newCap);
        if (ret != ARCHIVE_OK) {
            return ret;
        }
    }

    ret = safeMemoryCopy(mem->buffer + mem->position, mem->capacity - mem->position, buf, len);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    mem->position += len;
    if (mem->position > mem->size) {
        mem->size = mem->position;
    }

    return len;
}

LOCAL int MemStreamClose(struct Stream *stream)
{
    if (stream == NULL) {
        return ARCHIVE_OK;
    }
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;

    if (mem) {
        if (mem->buffer) {
            free(mem->buffer);
            mem->buffer = NULL;
        }
    }

    return ARCHIVE_OK;
}

LOCAL int MemStreamSeek(struct Stream *stream, int64_t offset, int origin)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;
    int64_t newPos;
    switch (origin) {
        case SEEK_SET:
            newPos = offset;
            break;
        case SEEK_CUR:
            newPos = (int64_t)mem->position + offset;
            break;
        case SEEK_END:
            newPos = (int64_t)mem->size + offset;
            break;
        default:
            return ARCHIVE_SEEK_ERROR;
    }

    if (newPos < 0 || (size_t)newPos > mem->capacity) {
        return ARCHIVE_SEEK_ERROR;
    }

    mem->position = (size_t)newPos;

    return ARCHIVE_OK;
}


LOCAL int64_t MemStreamTell(struct Stream *stream)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;

    return mem->position;
}

LOCAL const StreamOps g_MemStreamOps = {
    .open = MemStreamOpen,
    .read = MemStreamRead,
    .write = MemStreamWrite,
    .seek = MemStreamSeek,
    .tell = MemStreamTell,
    .close = MemStreamClose,
};

ARCHIVE_IMPL struct Stream *MemStreamCreate(void)
{
    MemStream *mem = calloc(1, sizeof(MemStream));
    if (mem == NULL) {
        ARCHIVE_ERR("MemStreamCreate fail!\n");
        return NULL;
    }
    SET_OBJ_MAGIC(&mem->stream, STREAM_MAGIC_MEM);

    mem->stream.ops = g_MemStreamOps;
    mem->memInitSize = MEM_INIT_SIZE;

    return (struct Stream *)mem;
}

ARCHIVE_IMPL int32_t MemStreamGetBufferAt(struct Stream *stream, size_t position, const void **buf)
{
    ASSERT(stream != NULL);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;
    if (!buf || !mem->buffer || mem->size < position)
        return ARCHIVE_SEEK_ERROR;
    *buf = mem->buffer + position;
    return ARCHIVE_OK;
}

ARCHIVE_IMPL int MemStreamSetInitSize(struct Stream *stream, size_t size)
{
    ASSERT(stream);
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_MEM);

    MemStream *mem = (MemStream *)stream;
    mem->memInitSize = size;
    return ARCHIVE_OK;
}
