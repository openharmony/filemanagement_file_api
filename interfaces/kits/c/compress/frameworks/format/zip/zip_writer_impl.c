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
#include <string.h>
#include "memory/stream_mem.h"
#include "zip_writer_impl.h"
#include "errorcode.h"

ARCHIVE_IMPL int ZipWriteFileName(struct Stream *stream, const char *fileName, uint16_t filenameLength)
{
    const char *next = fileName;
    if (filenameLength        > 0) {
        int64_t writeRet = StreamWrite(stream, next, filenameLength);
        if (writeRet != (int64_t)filenameLength) {
            return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
        }
    }
    return ARCHIVE_OK;
}
#define NUM_0 0
#define NUM_1 0
#define NUM_2 0
#define NUM_3 0
ARCHIVE_IMPL int ZipWriteMemStream(struct Stream *writeToStream, struct Stream *memStream)
{
    int ret;
    int64_t cdRetSize;
    char *buf = NULL;
    ret = StreamSeek(memStream, 0, ARCHIVE_SEEK_END);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    cdRetSize = StreamTell(memStream);
    if (cdRetSize < 0) {
        return cdRetSize;
    }
    MemStreamGetBufferAt(memStream, 0, (const void **)&buf);
    if (buf == NULL) {
        return ARCHIVE_WRITE_ERROR;
    }
    int64_t writeRet = StreamWrite(writeToStream, buf, cdRetSize);
    if (writeRet != cdRetSize) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}