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
#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>
#include <stddef.h>

#include "stream.h"
#ifdef __cplusplus
extern "C" {
#endif

#define STREAM_MODE_RD 0x1
#define STREAM_MODE_WR 0x2

#define STREAM_TOTAL_IN 0x3
#define STREAM_TOTAL_OUT 0x4

struct Stream;

typedef int (*StreamOpenFunc)(struct Stream *stream, int mode);

typedef int64_t (*StreamReadFunc)(struct Stream *stream, void *buf, int64_t size);

typedef int64_t (*StreamWriteFunc)(struct Stream *stream, const void *buf, int64_t size);

typedef int (*StreamSeekFunc)(struct Stream *stream, int64_t offset, int origin);

typedef int64_t (*StreamTellFunc)(struct Stream *stream);

typedef int (*StreamCloseFunc)(struct Stream *stream);

typedef struct {
    StreamOpenFunc open;
    StreamReadFunc read;
    StreamWriteFunc write;
    StreamSeekFunc seek;
    StreamTellFunc tell;
    StreamCloseFunc close;
} StreamOps;

struct Stream {
    unsigned int magic;
    uint64_t totalIn;
    uint64_t totalOut;
    uint64_t maxTotalIn;
    uint64_t headerSize;
    struct Stream *base;
    StreamOps ops;
};

int StreamOpen(struct Stream *stream, int mode);

int64_t StreamRead(struct Stream *stream, void *buf, int64_t size);

int64_t StreamWrite(struct Stream *stream, const void *buf, int64_t size);

int StreamSeek(struct Stream *stream, int64_t offset, int origin);

int64_t StreamTell(struct Stream *stream);

int StreamClose(struct Stream *stream);

void StreamDestroy(struct Stream **stream);

void StreamSetBase(struct Stream *stream, struct Stream *base);

uint64_t StreamGetTotalIn(struct Stream *stream);

uint64_t StreamGetTotalOut(struct Stream *stream);

uint32_t StreamGetHeaderSize(struct Stream *stream);

void StreamSetMaxTotalIn(struct Stream *stream, uint64_t maxTotalIn);

int StreamWriteUint8(struct Stream *stream, uint8_t value);

int StreamWriteUint16(struct Stream *stream, uint16_t value);

int StreamWriteUint32(struct Stream *stream, uint32_t value);

int StreamWriteUint64(struct Stream *stream, uint64_t value);

int StreamGetValueInt64(struct Stream *stream, int32_t prop, int64_t *value);

#ifdef __cplusplus
}
#endif


#endif