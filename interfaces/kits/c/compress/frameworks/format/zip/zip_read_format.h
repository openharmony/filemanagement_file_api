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

#ifndef ZIP_READ_FORMAT_H
#define ZIP_READ_FORMAT_H

#include "zip_format.h"

#pragma pack(1)

struct ReadCentralDirHeader {
    uint16_t versionMadeBy;
    uint16_t versionNeeded;
    uint16_t flag;
    uint16_t compressionMethod;
    uint32_t dosModDateTime;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength;
    uint16_t fileCommentLength;
    uint16_t diskNumberStart;
    uint16_t internalFileAttr;
    uint32_t externalFileAttr;
    uint32_t relativeOffsetOfLocalHeader;
};

struct ReadZip64ExtraField {
    uint64_t uncompressedSize;
    uint64_t compressedSize;
    uint64_t relativeOffsetOfLocalHeader;
    uint32_t diskNumberStart;
};

#pragma pack()

#endif