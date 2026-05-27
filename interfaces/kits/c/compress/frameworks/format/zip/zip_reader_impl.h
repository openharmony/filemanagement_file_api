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

#ifndef ZIP_READ_IMPL_H
#define ZIP_READ_IMPL_H

#include "stream.h"
#include "zip_read_format.h"

#ifdef __cplusplus
extern "C" {
#endif

int ZipCheckSignature(struct Stream *stream, uint32_t check);
int ZipSearchEOCD(struct Stream *stream, struct EndOfCentralDir *eocd, uint64_t *offsetOfEOCD,
    struct EndOfCentralDir64 *eocd64, uint64_t *offsetOfEOCD64, int8_t *isZip64);
int ZipCheckAndCorrectCentralDirOffset(struct Stream *stream, uint64_t offsetOfEOCD,
    uint8_t isZip64, uint64_t *offsetOfCentralDir, uint64_t *sizeOfCentralDir, int64_t *offsetCorrection);
int ZipReadZip64ExtraField(void *extraField, uint16_t cur, uint16_t dataSize,
    struct ReadZip64ExtraField *field);
int ZipReadUnicodePathExtraField(void *extraField, uint16_t cur, uint16_t dataSize,
    unsigned char *fileName, size_t fileNameSize);
int ZipReadExtraFieldBuffer(struct Stream *stream, size_t extraFieldPos, void *extraField,
    uint16_t extraFieldLength);
int ZipReadCentralDirHeader(struct Stream *stream, uint64_t position,
    struct ReadCentralDirHeader *centralDirHeader);
uint16_t ZipReadGetPkVerify(uint32_t date, uint64_t crc, uint16_t flag);
int ZipSeekAndCheckLocalHeader(struct Stream *stream, uint64_t relativeOffsetOfLocalHeader,
    int64_t offsetCorrection);

#ifdef __cplusplus
}
#endif

#endif // ZIP_READ_IMPL_H