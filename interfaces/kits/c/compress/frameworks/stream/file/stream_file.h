
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
#ifndef STREAM_FILE_H
#define STREAM_FILE_H
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#define OPEN_MODE_READ          (0x01)
#define OPEN_MODE_WRITE         (0x02)
#define OPEN_MODE_READ_WRITE    (OPEN_MODE_READ | OPEN_MODE_WRITE)
#define OPEN_MODE_APPEND        (0x04)
#define OPEN_MODE_CREATE        (0x08)
#define OPEN_MODE_EXISTING      (0x10)
struct Stream *FileStreamCreate(const char *path);
struct Stream *FileHandleStreamCreate(FILE *handle);
int FileStreamSetPath(struct Stream *stream, const char *path);
const char *FindLastSeparator(const char *filePath);
int GetParentDirectory(const char *filePath, char *parentPath, size_t parentPathSize);
#ifdef __cplusplus
}
#endif

#endif //FILEMANAGEMENT_FILE_API_STREAM_FILE_H
