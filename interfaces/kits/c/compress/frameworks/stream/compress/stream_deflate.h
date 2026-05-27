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
#ifndef STREAM_DEFLATE_H
#define STREAM_DEFLATE_H

#include "stream.h"
#include "zlib.h"
#include "archive_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Stream *DeflateStreamCreate(void);

struct Stream *DeflateStreamCreateSpec(int method, int level, int windowBits, int memLevel, int strategy);

int GetStreamDataType(struct Stream *stream);

ARCHIVE_IMPL z_stream *DeflateGetZStream(struct Stream *stream);

#ifdef __cplusplus
}
#endif

#endif