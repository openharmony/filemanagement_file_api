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

#include "archive_macros.h"
#include "zip_reader_impl.h"
#include <string.h>
#include <stdbool.h>
#include "zip_handler_common.h"
#include "securec.h"
#include "errorcode.h"
#include "zlib.h"

#define OFFSET_ADJUSTMENT_1 1
#define OFFSET_ADJUSTMENT_2 2
#define OFFSET_ADJUSTMENT_3 3
#define OFFSET_ADJUSTMENT_4 4

ARCHIVE_IMPL int ZipCheckSignature(struct Stream *stream, uint32_t check)
{
    if (stream == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    uint32_t sigRead;
    size_t read = StreamRead(stream, &sigRead, ZIP_SIZE_SIGNATURE);
    if (read != ZIP_SIZE_SIGNATURE) {
        return ARCHIVE_STREAM_ERROR;
    }

    if (sigRead != check) {
        return ARCHIVE_FORMAT_ERROR;
    }
    return ARCHIVE_OK;
}

LOCAL int ZipReadEOCD(struct EndOfCentralDir *eocd, unsigned char *buf, int pos)
{
    (void)memcpy_s(eocd, sizeof(struct EndOfCentralDir), buf + pos, sizeof(struct EndOfCentralDir));
    if (GETV(eocd->totalNumberOfEntriesOnThisDisk) != GETV(eocd->totalNumberOfEntries)) {
        return ARCHIVE_FORMAT_ERROR;
    }

    return ARCHIVE_OK;
}

LOCAL int ZipReadEOCD64(struct Stream *stream, struct EndOfCentralDir64 *eocd64,
    uint64_t *offsetOfEOCD64, unsigned char *buf, int pos)
{
    struct EndOfCentralDirLocator64 eocdLocator64;
    (void)memcpy_s(&eocdLocator64, sizeof(struct EndOfCentralDirLocator64), buf + pos,
        sizeof(struct EndOfCentralDirLocator64));
    *offsetOfEOCD64 = GETV(eocdLocator64.offsetOfEOCD64);
    int ret = StreamSeek(stream, *offsetOfEOCD64, SEEK_SET);
    RETURN_IF_FAIL(ret);

    size_t read = StreamRead(stream, eocd64, sizeof(struct EndOfCentralDir64));
    if (read != sizeof(struct EndOfCentralDir64)) {
        return ARCHIVE_FORMAT_ERROR;
    }

    if (GETV(eocd64->signature) != ZIP_MAGIC_ENDHEADER64) {
        return ARCHIVE_FORMAT_ERROR;
    }

    if (GETV(eocd64->totalNumberOfEntriesOnThisDisk) != GETV(eocd64->totalNumberOfEntries)) {
        return ARCHIVE_FORMAT_ERROR;
    }

    return ARCHIVE_OK;
}

LOCAL int ZipSearchEOCD64(struct Stream *stream, struct EndOfCentralDir64 *eocd64, uint64_t *offsetOfEOCD64,
                          unsigned char *buffer, int cur)
{
    int ret;
    if (cur < ZIP_SIZE_CD_LOCATOR64) {
        return ARCHIVE_FORMAT_ERROR;
    }

    if (GetLittleEndianValue(buffer + cur - ZIP_SIZE_CD_LOCATOR64, ZIP_SIZE_SIGNATURE) ==
        ZIP_MAGIC_ENDLOCHEADER64) {
        ret = ZipReadEOCD64(stream, eocd64, offsetOfEOCD64, buffer, cur - ZIP_SIZE_CD_LOCATOR64);
        RETURN_IF_FAIL(ret);
    }
    return ARCHIVE_OK;
}

static int ZipSearchEOCDFromback(struct Stream *stream, unsigned char *buffer, int64_t maxRead,
    struct EndOfCentralDir *eocd, uint64_t *offsetOfEOCD, struct EndOfCentralDir64 *eocd64,
    uint64_t *offsetOfEOCD64, int8_t *isZip64, int64_t fileSize)
{
    for (int64_t cur = maxRead - ZIP_SIZE_EOCD_FIXED; cur >= 0;) {
        switch (buffer[cur]) {
            case 0x50:
                if (GetLittleEndianValue(buffer + cur, ZIP_SIZE_SIGNATURE) != ZIP_MAGIC_ENDHEADER) {
                    cur -= OFFSET_ADJUSTMENT_4;
                    break;
                }

                int ret = ZipReadEOCD(eocd, buffer, cur);
                RETURN_IF_FAIL(ret);
                *offsetOfEOCD = fileSize - (maxRead - cur);
                if (GETV(eocd->totalNumberOfEntries) != UINT16_MAX &&
                    GETV(eocd->offsetOfCentralDir) != UINT32_MAX) {
                    return ARCHIVE_OK;
                }
                *isZip64 = 1;
                return ZipSearchEOCD64(stream, eocd64, offsetOfEOCD64, buffer, cur);
            case 0x4b:
                cur -= OFFSET_ADJUSTMENT_1;
                break;
            case 0x05:
                cur -= OFFSET_ADJUSTMENT_2;
                break;
            case 0x06:
                cur -= OFFSET_ADJUSTMENT_3;
                break;
            default:
                cur -= OFFSET_ADJUSTMENT_4;
                break;
        }
    }
    return ARCHIVE_FORMAT_ERROR;
}

ARCHIVE_IMPL int ZipSearchEOCD(struct Stream *stream, struct EndOfCentralDir *eocd, uint64_t *offsetOfEOCD,
    struct EndOfCentralDir64 *eocd64, uint64_t *offsetOfEOCD64, int8_t *isZip64)
{
    if (stream == NULL || eocd == NULL || offsetOfEOCD == NULL || eocd64 == NULL ||
        offsetOfEOCD64 == NULL || isZip64 == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    int ret;
    int64_t maxRead = ZIP_SIZE_MAX_GLOBAL_COMMENT + ZIP_SIZE_EOCD_FIXED + ZIP_SIZE_CD_LOCATOR64;
    ret = StreamSeek(stream, 0, ARCHIVE_SEEK_END);
    RETURN_IF_FAIL(ret);
    int64_t fileSize = StreamTell(stream);
    maxRead = fileSize < maxRead ? fileSize : maxRead;

    unsigned char *buffer = (unsigned char *)malloc(maxRead * sizeof(unsigned char));
    if (buffer == NULL) {
        return ARCHIVE_MEM_ERROR;
    }

    ret = StreamSeek(stream, -maxRead, ARCHIVE_SEEK_END);
    if (ret != ARCHIVE_OK) {
        free(buffer);
        return ret;
    }

    size_t read = StreamRead(stream, buffer, maxRead);
    if ((int64_t)read != maxRead) {
        free(buffer);
        return ARCHIVE_STREAM_ERROR;
    }

    ret = ZipSearchEOCDFromback(stream, buffer, maxRead, eocd, offsetOfEOCD, eocd64, offsetOfEOCD64, isZip64, fileSize);
    free(buffer);
    return ret;
}

ARCHIVE_IMPL int ZipCheckAndCorrectCentralDirOffset(struct Stream *stream, uint64_t offsetOfEOCD,
    uint8_t isZip64, uint64_t *offsetOfCentralDir, uint64_t *sizeOfCentralDir, int64_t *offsetCorrection)
{
    if (stream == NULL || offsetOfCentralDir == NULL || sizeOfCentralDir == NULL ||
        offsetCorrection == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    int ret;
    *offsetCorrection = 0;
    ret = StreamSeek(stream, *offsetOfCentralDir, SEEK_SET);
    RETURN_IF_FAIL(ret);

    if (ZipCheckSignature(stream, ZIP_MAGIC_CENTRALHEADER) == ARCHIVE_OK) {
        if (offsetOfEOCD < *offsetOfCentralDir + *sizeOfCentralDir) {
            *sizeOfCentralDir = offsetOfEOCD - *offsetOfCentralDir;
        }
    } else {
        if (offsetOfEOCD > UINT32_MAX && isZip64 == 0) {
            return ARCHIVE_FORMAT_ERROR;
        }
        ret = StreamSeek(stream, offsetOfEOCD - *sizeOfCentralDir, SEEK_SET);
        RETURN_IF_FAIL(ret);
        if (ZipCheckSignature(stream, ZIP_MAGIC_CENTRALHEADER) == ARCHIVE_OK) {
            uint64_t tmp = *offsetOfCentralDir;
            *offsetOfCentralDir = offsetOfEOCD - *sizeOfCentralDir;
            *offsetCorrection = *offsetOfCentralDir - tmp;
        } else if (*sizeOfCentralDir == 0) {
            // empty zip file
        } else {
            return ARCHIVE_FORMAT_ERROR;
        }
    }

    if (offsetOfEOCD < *offsetOfCentralDir || *offsetCorrection < 0) {
        return ARCHIVE_FORMAT_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int ZipReadZip64ExtraField(void *extraField, uint16_t cur, uint16_t dataSize,
    struct ReadZip64ExtraField *field)
{
    if (extraField == NULL || field == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    uint16_t pos = 0;
    if (pos >= dataSize) {
        return ARCHIVE_OK;
    }

    if (field->uncompressedSize == UINT32_MAX) {
        field->uncompressedSize = GetLittleEndianValue(extraField + cur + pos, sizeof(uint64_t));
        pos += sizeof(uint64_t);
    }

    if (pos >= dataSize) {
        return ARCHIVE_OK;
    }

    if (field->compressedSize == UINT32_MAX) {
        field->compressedSize = GetLittleEndianValue(extraField + cur + pos, sizeof(uint64_t));
        pos += sizeof(uint64_t);
    }

    if (pos >= dataSize) {
        return ARCHIVE_OK;
    }

    if (field->relativeOffsetOfLocalHeader == UINT32_MAX) {
        field->relativeOffsetOfLocalHeader = GetLittleEndianValue(extraField + cur + pos, sizeof(uint64_t));
        pos += sizeof(uint64_t);
    }

    if (pos >= dataSize) {
        return ARCHIVE_OK;
    }

    if (field->diskNumberStart == UINT16_MAX) {
        field->diskNumberStart = *((uint32_t *)extraField + cur + pos);
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int ZipReadUnicodePathExtraField(void *extraField, uint16_t cur, uint16_t dataSize,
    unsigned char *fileName, size_t filenameSize)
{
    if (extraField == NULL || fileName == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    uint16_t pos = 0;
    uint8_t version = GetLittleEndianValue(extraField + cur, sizeof(uint8_t));
    if (version != 1) {
        return ARCHIVE_FORMAT_ERROR;
    }
    pos += sizeof(version);
    uint32_t fileNameCrc = GetLittleEndianValue(extraField + cur + pos, sizeof(uint32_t));
    if (fileNameCrc != crc32(0, fileName, filenameSize)) {
        return ARCHIVE_CRC_ERROR;
    }
    pos += sizeof(fileNameCrc);

    uint16_t fileNameSize = dataSize - pos;
    int ret = memcpy_s(fileName, ZIP_FILE_NAME_LEN_MAX, extraField + cur + pos, fileNameSize);
    fileNameSize = fileNameSize > ZIP_FILE_NAME_LEN_MAX ? ZIP_FILE_NAME_LEN_MAX : fileNameSize;
    fileName[fileNameSize] = '\0';

    if (ret != EOK) {
        return ARCHIVE_MEM_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int ZipReadExtraFieldBuffer(struct Stream *stream, size_t extraFieldPos, void *extraField,
    uint16_t extraFieldLength)
{
    int ret;
    size_t read;
    if (extraField == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    if (extraFieldLength == 0) {
        return ARCHIVE_OK;
    }

    ret = StreamSeek(stream, extraFieldPos, ARCHIVE_SEEK_SET);
    RETURN_IF_FAIL(ret);

    read = StreamRead(stream, extraField, extraFieldLength);
    if (read != extraFieldLength) {
        return ARCHIVE_READ_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int ZipReadCentralDirHeader(struct Stream *stream, uint64_t position,
    struct ReadCentralDirHeader *centralDirHeader)
{
    if (stream == NULL || centralDirHeader == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    int ret;
    size_t read;
    uint32_t signature;
    ret = StreamSeek(stream, position, ARCHIVE_SEEK_SET);
    RETURN_IF_FAIL(ret);
    read = StreamRead(stream, &signature, sizeof(signature));
    if (read != sizeof(signature)) {
        return ARCHIVE_FORMAT_ERROR;
    }

    if (signature != ZIP_MAGIC_CENTRALHEADER) {
        if (signature == ZIP_MAGIC_ENDHEADER || signature == ZIP_MAGIC_ENDHEADER64) {
            return ARCHIVE_END_OF_LIST;
        }
        return ARCHIVE_FORMAT_ERROR;
    }
    read = StreamRead(stream, centralDirHeader, sizeof(*centralDirHeader));
    if (read != sizeof(*centralDirHeader)) {
        return ARCHIVE_FORMAT_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL uint16_t ZipReadGetPkVerify(uint32_t date, uint64_t crc, uint16_t flag)
{
    if (flag & ZIP_FLAG_DATA_DESCRIPTOR) {
        return (((date >> BITS_SHIFT_16) &0xff) << BITS_SHIFT_8 | ((date >> BITS_SHIFT_8) & 0xff));
    }
    return ((crc >> BITS_SHIFT_16) & 0xff) << BITS_SHIFT_8 | ((crc >> BITS_SHIFT_24) & 0xff);
}

ARCHIVE_IMPL int ZipSeekAndCheckLocalHeader(struct Stream *stream, uint64_t relativeOffsetOfLocalHeader,
    int64_t offsetCorrection)
{
    int ret = ARCHIVE_OK;
    uint64_t offset = 0;
    bool needCorrect = 0;
    if (offsetCorrection > 0 && relativeOffsetOfLocalHeader <= UINT64_MAX - (uint64_t)offsetCorrection) {
        ret = StreamSeek(stream, relativeOffsetOfLocalHeader + offsetCorrection, ARCHIVE_SEEK_SET);
        if (ret == ARCHIVE_OK && ZipCheckSignature(stream, ZIP_MAGIC_LOCALHEADER) == ARCHIVE_OK) {
            offset = relativeOffsetOfLocalHeader + (uint64_t)offsetCorrection;
            needCorrect = true;
        }
    }

    if (!needCorrect) {
        ret = StreamSeek(stream, relativeOffsetOfLocalHeader, ARCHIVE_SEEK_SET);
        if (ret == ARCHIVE_OK && ZipCheckSignature(stream, ZIP_MAGIC_LOCALHEADER) == ARCHIVE_OK) {
            offset = relativeOffsetOfLocalHeader;
            needCorrect = false;
        } else {
            ret = ARCHIVE_FORMAT_ERROR;
        }
    }
    RETURN_IF_FAIL(ret);
    return StreamSeek(stream, offset, ARCHIVE_SEEK_SET);
}