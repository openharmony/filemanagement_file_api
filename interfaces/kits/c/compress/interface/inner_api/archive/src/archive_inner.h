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

#ifndef ARCHIVE_INNER_H
#define ARCHIVE_INNER_H

#include "oh_archive_errcode.h"
#include "archive_macros.h"
#include <stdbool.h>

typedef enum {
    ARCHIVE_FMT_ZIP = 0,
    ARCHIVE_FMT_GZIP,
    ARCHIVE_FMT_7ZIP,
    ARCHIVE_FMT_RAR,
    ARCHIVE_FMT_INVALID
} ArchiveFormat;

struct HmArchiveS {
    uint32_t magic;
    ArchiveFormat fmt;
};

typedef struct {
    int ret;
    OH_Archive_ErrCode errcode;
} ErrCodeMap;

int CreateDirectory(const char *path, const char *base);
int CreateSymlink(const char *old, const char *newSym);
int CreateParentDirectory(const char *filePath, const char* base);
int GetOutputFilePath(const char *fileName, const char *outDir, char *outPath,
    size_t size);
uint8_t IsUTF8Str(char *fStr);
char *ConvertStrGBKToUTF8(const char *str);
int IsFileExists(const char *fileName);
int GenerateNewFilename(const char *originalPath, char *newPath, size_t newPathSize);
int IsSymlinkExists(const char *fileName);
bool IsUtf8Encoded(unsigned char *data, bool skipTwoBytes);
#endif