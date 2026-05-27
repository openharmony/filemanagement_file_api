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
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>
#include <sched.h>
#include <stdatomic.h>
#include <unistd.h>

#include "zip_util.h"
#include "zip_reader_impl.h"
#include "archive_macros.h"
#include "archive_inner.h"
#include "stream.h"
#include "file/stream_file.h"
#include "compress/stream_deflate.h"
#include "raw/stream_raw.h"
#include "zlib.h"
#include "zip_handler_common.h"
#include "securec.h"
#include "errorcode.h"
#include "oh_archive.h"

#define EXTRACT_ENTRY_CONTINUE (0)
#define EXTRACT_ENTRY_BREAK (-1)

struct ZipReaderEntryInfo {
    uint64_t uncompressedSize;
    uint64_t compressedSize;
    uint64_t relativeOffsetOfLocalHeader;
    uint32_t diskNumberStart;

    uint16_t compressionMethod;

    uint16_t aesVersion;
    uint8_t aesStrength;
    uint16_t pkVerify;
    bool isEncrypted;
    uint32_t entryCrc;

    time_t modifiedTime;

    bool useUnicodePath;
    char entryName[ZIP_FILE_NAME_LEN_MAX + 1];
};

struct ZipReaderExtractContext {
    struct ZipReader *reader;
    const char *outDir;
    size_t fileIndex;
    bool entryOpened;
    struct ReadCentralDirHeader centralDirHeader;
    struct LocalFileHeader localFileHeader;
    struct ZipReaderEntryInfo entryInfo;
    struct StrongEncryptionHeader strongEncryptHeader;
    // progress of decompress each entry
    uint64_t progressPrevious;
    uint64_t progressUpdated;
};

struct ReadResource {
    struct Stream *baseStream;
    struct Stream *compressStream;
    struct Stream *extractStream;
    char buffer[ZIP_BUF_INTERNAL];
};

struct ZipReader {
    struct EndOfCentralDir endOfCentralDir;
    struct EndOfCentralDir64 endOfCentralDir64;

    struct ReadResource resource;
    struct ZipReaderExtractContext context;

    uint64_t offsetOfEOCD;
    uint64_t offsetOfEOCD64;
    uint64_t offsetOfCentralDir; /* offset of start of central directory */
    int64_t offsetCorrection;
    uint64_t sizeOfCentralDir;
    uint64_t curCentralDirHeaderPos;
    int8_t isZip64;
    uint64_t totalNumberOfEntries;

    // progress
    OH_Archive_ProgressHandlerWithData progressHandlerWithData;
    void *progressHandlerData;
    uint64_t progressTotal;
    _Atomic uint64_t progressUpdated;
};

/* 删除当前正在解压Item发生异常生成的文件*/
static void DeleteTempFileWithRootPath(const char *rootPath, const char *file)
{
    if (remove(file) != 0) {
        return;
    }

    // 判断Item解压生居的文件夹是否需要删除，如果每一级文件夹内都没有其他文件，则删除父目录文件夹，至指定root目录；
    char parentPath[ZIP_FILE_NAME_LEN_MAX];
    if (GetParentDirectory(file, parentPath, ZIP_FILE_NAME_LEN_MAX) != ARCHIVE_OK) {
        return;
    }

    while (strlen(rootPath) < strlen(parentPath)) {
        // 删除文件夹失败则表明文件夹内存在其他文件，退出
        if (remove(parentPath) != 0) {
            return;
        }

        char tempParentPath[ZIP_FILE_NAME_LEN_MAX];
        if (strncpy_s(tempParentPath, ZIP_FILE_NAME_LEN_MAX, parentPath, strlen(parentPath)) != EOK) {
            return;
        }

        if (GetParentDirectory(tempParentPath, parentPath, ZIP_FILE_NAME_LEN_MAX) != ARCHIVE_OK) {
            return;
        }
    }
}

LOCAL int ZipAttribWin32ToPosix(uint32_t win32Attrib, uint32_t *posixAttrib)
{
    if (posixAttrib == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    *posixAttrib = S_IRUSR | S_IRGRP | S_IROTH;

    if ((win32Attrib & 0x01) == 0) { // 0x01 FILE_ATTRIBUTE_READONLY
        *posixAttrib |= S_IWUSR | S_IWGRP | S_IWOTH;
    }

    if ((win32Attrib & 0x400) == 0x400) { // 0x400 FILE_ATTRIBUTE_REPARSE_POINT
        *posixAttrib |= S_IFLNK;
    } else if ((win32Attrib & 0x10) == 0x10) { // 0x10 FILE_ATTRIBUTE_DIRECTORY
        *posixAttrib |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    } else {
        *posixAttrib |= S_IFREG;
    }

    return ARCHIVE_OK;
}

LOCAL int ZipConvertAttrib(uint8_t srcHostSys, uint32_t srcAttrib, uint32_t *targetAttrib)
{
    if (!targetAttrib) {
        return ARCHIVE_PARAM_ERROR;
    }

    *targetAttrib = 0;

    if ((srcHostSys == ZIP_HOST_SYSTEM_MSDOS) || (srcHostSys == ZIP_HOST_SYSTEM_WINDOWS_NTFS)) {
        return ZipAttribWin32ToPosix(srcAttrib, targetAttrib);
    } else if ((srcHostSys == ZIP_HOST_SYSTEM_UNIX) || (srcHostSys == ZIP_HOST_SYSTEM_OSX_DARWIN) ||
               (srcHostSys == ZIP_HOST_SYSTEM_RISCOS)) {
        /* if high 16 bytes are set, it contains unix specifice attributes */
        if ((srcAttrib >> 16) != 0) {
            srcAttrib >>= 16;   // shift high 16 bytes
        }

        *targetAttrib = srcAttrib;
        return ARCHIVE_OK;
    }

    return ARCHIVE_SUPPORT_ERROR;
}

LOCAL bool ZipIsSymlink(uint32_t attrib, int32_t versionMadeBy)
{
    uint32_t posixAttrib = 0;
    uint8_t system = ZIP_HOST_SYSTEM(versionMadeBy);
    int ret = ZipConvertAttrib(system, attrib, &posixAttrib);
    if (ret == ARCHIVE_OK && (posixAttrib & 0170000) == 0120000) { // 0170000: mask code, 0120000: symbol link
        return true;
    }
    return false;
}

LOCAL struct ZipReader *ZipReaderCreate(void)
{
    struct ZipReader *reader = (struct ZipReader *)calloc(1, sizeof(struct ZipReader));
    if (reader == NULL) {
        return NULL;
    }

    reader->context.reader = reader;
    return reader;
}

LOCAL void ZipReaderDestroy(struct ZipReader **reader)
{
    free(*reader);
    *reader = NULL;
}

LOCAL int ZipReaderProcessZip64ExtraField(struct ZipReaderExtractContext *context, void *extraField, uint16_t cur,
    uint16_t dataSize)
{
    struct ReadZip64ExtraField field = {.uncompressedSize = GETV(context->centralDirHeader.uncompressedSize),
        .compressedSize = GETV(context->centralDirHeader.compressedSize),
        .relativeOffsetOfLocalHeader = GETV(context->centralDirHeader.relativeOffsetOfLocalHeader),
        .diskNumberStart = GETV(context->centralDirHeader.diskNumberStart)};

    int ret = ZipReadZip64ExtraField(extraField, cur, dataSize, &field);
    RETURN_IF_FAIL(ret);

    context->entryInfo.uncompressedSize = field.uncompressedSize;
    context->entryInfo.compressedSize = field.compressedSize;
    context->entryInfo.relativeOffsetOfLocalHeader = field.relativeOffsetOfLocalHeader;
    context->entryInfo.diskNumberStart = field.diskNumberStart;

    return ARCHIVE_OK;
}

LOCAL int ZipReaderProcessStrongEncryptExtraField(struct ZipReaderExtractContext *context, void *extraField,
    uint16_t cur, uint16_t dataSize)
{
    if (dataSize < sizeof(struct StrongEncryptionExtraFields)) {
        return ARCHIVE_READ_ERROR;
    }
    (void)memcpy_s((uint8_t *)(&context->strongEncryptHeader) + EXTRA_FIELD_HEADER_BYTES,
        sizeof(struct StrongEncryptionExtraFields), (uint8_t *)extraField + cur,
        sizeof(struct StrongEncryptionExtraFields));
    context->strongEncryptHeader.headerID = 0x0017;
    context->strongEncryptHeader.size = dataSize;
    return ARCHIVE_OK;
}

LOCAL int ZipReaderProcessExtraField(struct ZipReader *reader, void *extraField, uint16_t extraFieldLength)
{
    int ret = ARCHIVE_OK;
    uint16_t cur = 0;
    while (ret == ARCHIVE_OK && (cur + EXTRA_FIELD_HEADER_BYTES) <= extraFieldLength) {
        uint16_t headerID = GetLittleEndianValue(extraField + cur, sizeof(uint16_t));
        uint16_t dataSize = GetLittleEndianValue(extraField + cur + sizeof(uint16_t), sizeof(uint16_t));
        cur += EXTRA_FIELD_HEADER_BYTES;
        if (dataSize > extraFieldLength - cur) {
            dataSize = (uint16_t)(extraFieldLength - cur);
        }
        switch (headerID) {
            // Zip64 extended information extra field
            case 0x0001:
                ret = ZipReaderProcessZip64ExtraField(&reader->context, extraField, cur, dataSize);
                break;
            // Strong Encryption Header
            case 0x0017:
                ret = ZipReaderProcessStrongEncryptExtraField(&reader->context, extraField, cur, dataSize);
                break;
            // Info-ZIP Unicode Path extra field
            case 0x7075:
                if (dataSize < EXTRA_FIELD_UNICODE_PATH_MIN_LEN) {
                    break;
                }
                ret = ZipReadUnicodePathExtraField(extraField,
                    cur,
                    dataSize,
                    (unsigned char *)&reader->context.entryInfo.entryName,
                    GETV(reader->context.centralDirHeader.fileNameLength));
                if (ret == ARCHIVE_OK) {
                    reader->context.entryInfo.useUnicodePath = true;
                }
                break;
            default:
                break;
        }
        cur += dataSize;
    }
    return ret;
}

LOCAL int ZipReaderProcessCentralDirHeader(struct ZipReader *reader, uint64_t offsetOfCentralDirHeader)
{
    int ret = ZipReadCentralDirHeader(reader->resource.baseStream, offsetOfCentralDirHeader,
        &reader->context.centralDirHeader);
    RETURN_IF_FAIL(ret);
    reader->context.entryInfo.uncompressedSize = GETV(reader->context.centralDirHeader.uncompressedSize);
    reader->context.entryInfo.compressedSize = GETV(reader->context.centralDirHeader.compressedSize);
    reader->context.entryInfo.relativeOffsetOfLocalHeader =
        GETV(reader->context.centralDirHeader.relativeOffsetOfLocalHeader);
    reader->context.entryInfo.diskNumberStart = GETV(reader->context.centralDirHeader.diskNumberStart);
    reader->context.entryInfo.compressionMethod = GETV(reader->context.centralDirHeader.compressionMethod);
    reader->context.entryInfo.modifiedTime =
        ZipConvertDosDateToTime(GETV(reader->context.centralDirHeader.dosModDateTime));
    reader->context.entryInfo.aesVersion = 0;
    reader->context.entryInfo.aesStrength = 0;
    reader->context.entryInfo.pkVerify = 0;
    reader->context.entryInfo.isEncrypted = false;
    if (GETV(reader->context.centralDirHeader.flag) & ZIP_FLAG_ENCRYPTED) {
        reader->context.entryInfo.pkVerify =  ZipReadGetPkVerify(GETV(reader->context.centralDirHeader.dosModDateTime),
            GETV(reader->context.centralDirHeader.crc32),
            GETV(reader->context.centralDirHeader.flag));
        reader->context.entryInfo.isEncrypted = true;
    }
    reader->context.entryInfo.useUnicodePath = false;
    return ARCHIVE_OK;
}

LOCAL int ZipReaderConvertFileNameEncoding(struct ZipReader *reader, uint16_t fileNameLen)
{
    if (IsUTF8Str(reader->context.entryInfo.entryName)) {
        return ARCHIVE_OK;
    }
    uint16_t utf8NameSize = fileNameLen * 4 + 1; // 假设每个字符最多转换为4个字节，+1为结束符
    utf8NameSize = utf8NameSize > ZIP_FILE_NAME_LEN_MAX ? ZIP_FILE_NAME_LEN_MAX : utf8NameSize;
    char *utf8Name = ConvertStrGBKToUTF8((const char *)(reader->context.entryInfo.entryName));
    if (utf8Name) {
        int ret = strncpy_s(reader->context.entryInfo.entryName,  utf8NameSize, utf8Name, strlen(utf8Name));
        free(utf8Name);
        if (ret != EOK) {
            return ARCHIVE_INTERNAL_ERROR;
        }
        reader->context.entryInfo.entryName[utf8NameSize] = '\0';
    }

    return ARCHIVE_OK;
}

LOCAL void ZipReaderConvertPathSlash(struct ZipReader *reader)
{
    uint8_t hostOS = ZIP_HOST_SYSTEM(GETV(reader->context.centralDirHeader.versionMadeBy));
    if (hostOS != ZIP_HOST_OS_FAT && hostOS != ZIP_HOST_OS_NTFS && hostOS != ZIP_HOST_OS_HPFS &&
        hostOS != ZIP_HOST_OS_VFAT) {
            return;
    }
    if (strchr(reader->context.entryInfo.entryName, '\\') != NULL) {
        size_t len = strlen(reader->context.entryInfo.entryName);
        for (size_t i = 0; i < len; i++) {
            if (reader->context.entryInfo.entryName[i] == '\\') {
                reader->context.entryInfo.entryName[i] = '/';
            }
        }
    }
}

LOCAL int ZipReaderGoToCentralDirHeader(struct ZipReader *reader, uint64_t offsetOfCentralDirHeader)
{
    if (offsetOfCentralDirHeader >= reader->offsetOfCentralDir + reader->sizeOfCentralDir) {
        return ARCHIVE_END_OF_LIST;
    }
    int ret;
    ret = ZipReaderProcessCentralDirHeader(reader, offsetOfCentralDirHeader);
    RETURN_IF_FAIL(ret);
    uint16_t fileNameLen = GETV(reader->context.centralDirHeader.fileNameLength) < ZIP_FILE_NAME_LEN_MAX
                                ? GETV(reader->context.centralDirHeader.fileNameLength)
                                : ZIP_FILE_NAME_LEN_MAX;
    size_t read = StreamRead(reader->resource.baseStream, reader->context.entryInfo.entryName, fileNameLen);
    if (read != fileNameLen) {
        return ARCHIVE_READ_ERROR;
    }
    reader->context.entryInfo.entryName[fileNameLen] = '\0';
    void *extraField = malloc(GETV(reader->context.centralDirHeader.extraFieldLength));
    if (extraField == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    ret = ZipReadExtraFieldBuffer(reader->resource.baseStream,
        offsetOfCentralDirHeader + ZIP_SIZE_CD_ITEM + reader->context.centralDirHeader.fileNameLength,
        extraField,
        GETV(reader->context.centralDirHeader.extraFieldLength));
    if (ret != ARCHIVE_OK) {
        free(extraField);
        return ret;
    }

    ret = ZipReaderProcessExtraField(reader, extraField, GETV(reader->context.centralDirHeader.extraFieldLength));
    free(extraField);
    RETURN_IF_FAIL(ret);
    if (!reader->context.entryInfo.useUnicodePath &&
        (GETV(reader->context.centralDirHeader.flag) & ZIP_FLAG_UTF8) == 0) {
            ret = ZipReaderConvertFileNameEncoding(reader, fileNameLen);
            RETURN_IF_FAIL(ret);
        }
    ZipReaderConvertPathSlash(reader);
    return ret;
}

LOCAL int ZipReaderGotoFirstCentralDirHeader(struct ZipReader *reader)
{
    uint64_t offsetOfFirstCentralDirHeader = reader->offsetOfCentralDir;
    reader->context.fileIndex = 0;
    if (reader->context.fileIndex >= reader->totalNumberOfEntries) {
        return ARCHIVE_END_OF_LIST;
    }
    int ret = ZipReaderGoToCentralDirHeader(reader, offsetOfFirstCentralDirHeader);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    reader->curCentralDirHeaderPos = offsetOfFirstCentralDirHeader;
    return ARCHIVE_OK;
}

LOCAL int ZipReaderGotoNextCentralDirHeader(struct ZipReader *reader)
{
    uint64_t offsetOfNextCentralDirHeader =
        reader->curCentralDirHeaderPos + ZIP_SIZE_CD_ITEM + GETV(reader->context.centralDirHeader.fileNameLength) +
        GETV(reader->context.centralDirHeader.extraFieldLength) +
        GETV(reader->context.centralDirHeader.fileCommentLength);
    reader->context.fileIndex++;
    if (reader->context.fileIndex >= reader->totalNumberOfEntries) {
        return ARCHIVE_END_OF_LIST;
    }
    int ret = ZipReaderGoToCentralDirHeader(reader, offsetOfNextCentralDirHeader);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    reader->curCentralDirHeaderPos = offsetOfNextCentralDirHeader;
    return ARCHIVE_OK;
}

LOCAL int ZipReaderProcessEOCD(struct ZipReader *reader)
{
    int ret;
    // read end of central directory
    ret = ZipSearchEOCD(reader->resource.baseStream,
        &reader->endOfCentralDir,
        &reader->offsetOfEOCD,
        &reader->endOfCentralDir64,
        &reader->offsetOfEOCD64,
        &reader->isZip64);
    RETURN_IF_FAIL(ret);

    if (!reader->isZip64) {
        reader->offsetOfCentralDir =  GETV(reader->endOfCentralDir.offsetOfCentralDir);
        reader->sizeOfCentralDir = GETV(reader->endOfCentralDir.sizeOfCentralDir);
    } else {
        reader->offsetOfCentralDir = GETV(reader->endOfCentralDir64.offsetOfCentralDir);
        reader->sizeOfCentralDir = GETV(reader->endOfCentralDir64.sizeOfCentralDir);
    }
    ret = ZipCheckAndCorrectCentralDirOffset(reader->resource.baseStream,
        reader->isZip64 ? reader->offsetOfEOCD64 : reader->offsetOfEOCD,
        reader->isZip64,
        &reader->offsetOfCentralDir,
        &reader->sizeOfCentralDir,
        &reader->offsetCorrection);
    return ret;
}

LOCAL int ZipReaderOpenFile(HmArchiveReadInfo *archive, const char *infile)
{
    if (archive == NULL || infile == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret;

    struct ZipReader *reader = ZipReaderCreate();
    if (reader == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    archive->fmtInfo = reader;

    struct Stream *stream = FileStreamCreate(infile);
    if (stream == NULL) {
        ZipReaderDestroy(&reader);
        return ARCHIVE_STREAM_ERROR;
    }
    reader->resource.baseStream = stream;

    ret = StreamOpen(stream, ARCHIVE_OPEN_MODE_READ);
    if (ret != ARCHIVE_OK) {
        StreamDestroy(&stream);
        ZipReaderDestroy(&reader);
        return ret;
    }
    ret = ZipReaderProcessEOCD(reader);
    if (ret != ARCHIVE_OK) {
        StreamClose(stream);
        StreamDestroy(&stream);
        ZipReaderDestroy(&reader);
        return ret;
    }

    reader->curCentralDirHeaderPos = 0;
    reader->context.fileIndex = 0;
    reader->totalNumberOfEntries = reader->isZip64 ? GETV(reader->endOfCentralDir64.totalNumberOfEntries)
                                                   : GETV(reader->endOfCentralDir.totalNumberOfEntries);
    return ARCHIVE_OK;
}

LOCAL int64_t ZipReaderReadData(struct ZipReaderExtractContext *context, struct ReadResource *resource, void *buf,
    uint32_t len)
{
    if (!context->entryOpened) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    if (context->entryInfo.compressedSize == 0 || context->entryInfo.uncompressedSize == 0 ||
        resource->compressStream == NULL || len == 0) {
        return 0;
    }

    // Decompress the data using different methods depending on the compresssed stream
    size_t read = StreamRead(resource->compressStream, buf, len);
    if (read > UINT32_MAX) {
        return ARCHIVE_READ_ERROR;
    }

    // Update crc according to the data read
    context->entryInfo.entryCrc = crc32(context->entryInfo.entryCrc, buf, read);

    return read;
}

LOCAL int ZipReaderWriteData(struct ReadResource *resource, void *buf, uint32_t len)
{
    int64_t written = StreamWrite(resource->extractStream, buf, len);
    if (written != (int64_t)len) {
        return errno == ENOSPC ? ARCHIVE_NO_SPACE_ERROR : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}

LOCAL int ZipReaderProcessLocalHeader(struct ZipReaderExtractContext *context, struct ReadResource *resource)
{
    struct ZipReader *reader = context->reader;

    int ret;
    ret = ZipSeekAndCheckLocalHeader(resource->baseStream,
        context->entryInfo.relativeOffsetOfLocalHeader,
        reader->offsetCorrection);
    if (ret != ARCHIVE_OK) {
        return ARCHIVE_FORMAT_ERROR;
    }

    size_t read = StreamRead(resource->baseStream, &context->localFileHeader, sizeof(struct LocalFileHeader));
    if (read != sizeof(struct LocalFileHeader)) {
        return ARCHIVE_READ_ERROR;
    }

    // Skip reading the filenmae and extra filed ,and read them later when needed
    ret = StreamSeek(resource->baseStream,
        GETV(context->localFileHeader.fileNameLength) + GETV(context->localFileHeader.extraFieldLength),
        ARCHIVE_SEEK_CUR);
    RETURN_IF_FAIL(ret);

    return ARCHIVE_OK;
}

LOCAL int ZipReaderCloseEntry(struct ZipReaderExtractContext *context, struct ReadResource *resource)
{
    if (resource->compressStream != NULL) {
        StreamClose(resource->compressStream);
        StreamDestroy(&resource->compressStream);
        resource->compressStream = NULL;
    }

    if (resource->extractStream != NULL) {
        StreamClose(resource->extractStream);
        StreamDestroy(&resource->extractStream);
        resource->extractStream = NULL;
    }

    context->entryOpened = false;

    return ARCHIVE_OK;
}

LOCAL bool ZipReaderEntryIsSymlink(struct ReadCentralDirHeader *centralDirHeader)
{
    return ZipIsSymlink(GETV(centralDirHeader->externalFileAttr), GETV(centralDirHeader->versionMadeBy));
}

LOCAL bool ZipReaderEntryIsDirectory(struct ZipReaderExtractContext *context)
{
    size_t filenameLen = strlen(context->entryInfo.entryName);
    char *filename = context->entryInfo.entryName;
    if (filename != NULL && filenameLen > 0 && filename[filenameLen - 1] == '/') {
        return true;
    }

    uint8_t hostOS = ZIP_HOST_SYSTEM(GETV(context->centralDirHeader.versionMadeBy));
    if (GETV(context->centralDirHeader.uncompressedSize) == 0 && GETV(context->centralDirHeader.compressedSize) == 0 &&
       filename != NULL && filenameLen > 0 && filename[filenameLen - 1] == '\\') {
        switch (hostOS) {
            case ZIP_HOST_OS_FAT:
            case ZIP_HOST_OS_NTFS:
            case ZIP_HOST_OS_HPFS:
            case ZIP_HOST_OS_VFAT:
                return true;
            default:
                break;
        }
    }
    // 取高16位
    uint16_t highAttrib = (uint16_t)((GETV(context->centralDirHeader.externalFileAttr) >> 16) & 0xFFFF);
    switch (hostOS) {
        case ZIP_HOST_OS_AMIGA:
            switch (highAttrib & 06000) { // 06000 AmigaAttrib::kIFMT
                case 04000: // 04000 AmigaAttrib::KIFDIR
                    return true;
                case 02000: // 02000 AmigaAttrib::KIFREG
                    return false;
                default:
                    return false;
            }
        case ZIP_HOST_OS_FAT:
        case ZIP_HOST_OS_NTFS:
        case ZIP_HOST_OS_HPFS:
        case ZIP_HOST_OS_VFAT:
            return ((GETV(context->centralDirHeader.externalFileAttr) & 0x0010) != 0); // 0x0010 ATTRIBUTE_DIRECTORY
        case ZIP_HOST_OS_ATARI:
        case ZIP_HOST_OS_MAC:
        case ZIP_HOST_OS_VMS:
        case ZIP_HOST_OS_VM_CMS:
        case ZIP_HOST_OS_ACORN:
        case ZIP_HOST_OS_MVS:
            return false;
        case ZIP_HOST_OS_UNIX:
            return ((highAttrib & 00170000) == 0040000); // 00170000 is mask, 0040000 is directory, S_ISDIR
        default:
            return false;
    }
}

LOCAL int ZipReaderInitStream(struct ZipReaderExtractContext *context, struct ReadResource *resource,
    const char *outPath)
{
    int ret;
    switch (context->entryInfo.compressionMethod) {
        case OH_ARCHIVE_NO_COMPRESSION:
            resource->compressStream = RawStreamCreate();
            break;
        case OH_ARCHIVE_COMPRESS_DEFLATE:
            resource->compressStream = DeflateStreamCreate();
            break;
        default:
            return ARCHIVE_SUPPORT_ERROR;
    }

    if (resource->compressStream == NULL) {
        return ARCHIVE_MEM_ERROR;
    }

    StreamSetBase(resource->compressStream, resource->baseStream);
    ret = StreamOpen(resource->compressStream, ARCHIVE_OPEN_MODE_READ);
    RETURN_IF_FAIL(ret);
    if (!ZipReaderEntryIsSymlink(&context->centralDirHeader)) {
        resource->extractStream = FileStreamCreate(outPath);
        ret = StreamOpen(resource->extractStream, ARCHIVE_OPEN_MODE_CREATE);
    }
    return ret;
}

// Opens the entry of a compressed file for reading
LOCAL int ZipReaderOpenEntry(struct ZipReaderExtractContext *context, struct ReadResource *resource,
    const char *outPath)
{
    int ret;
    if (context->entryOpened) {
        ZipReaderCloseEntry(context, resource);
    }

    ret = ZipReaderProcessLocalHeader(context, resource);
    RETURN_IF_FAIL(ret);
    ret = ZipReaderInitStream(context, resource, outPath);
    if (ret != ARCHIVE_OK) {
        ZipReaderCloseEntry(context, resource);
        return ret;
    }

    if (context->entryInfo.compressionMethod == OH_ARCHIVE_NO_COMPRESSION) {
        StreamSetMaxTotalIn(resource->baseStream, context->entryInfo.compressedSize);
        uint32_t headerSize = StreamGetHeaderSize(resource->baseStream);
        StreamSetMaxTotalIn(resource->compressStream, context->entryInfo.compressedSize - headerSize);
    }

    ret = memset_s(resource->buffer, sizeof(resource->buffer), 0, sizeof(resource->buffer));
    if (ret != EOK) {
        ZipReaderCloseEntry(context, resource);
        return ARCHIVE_MEM_ERROR;
    }
    context->entryOpened = true;
    context->entryInfo.entryCrc = 0;
    return ARCHIVE_OK;
}

LOCAL int ZipReaderVerifyEntryCrc(struct ZipReaderExtractContext *context, struct ReadResource *resource)
{
    if (context->entryInfo.aesVersion <= 0x0001) {
        if (resource->compressStream == NULL) {
            return ARCHIVE_INTERNAL_ERROR;
        }
        size_t totalIn = StreamGetTotalIn(resource->compressStream);
        if (context->entryInfo.isEncrypted) {
            totalIn += StreamGetHeaderSize(resource->baseStream);
        }
        if (context->entryInfo.compressedSize <= totalIn &&
            context->entryInfo.entryCrc != GETV(context->centralDirHeader.crc32)) {
            return ARCHIVE_CRC_ERROR;
        }
    }
    return ARCHIVE_OK;
}

LOCAL int ZipReaderAutoRenamePath(struct ZipReader *reader, const char *entryName,
    const char *outPath, char *newFilePath, size_t newFilePathSize)
{
    int ret;
    ret = strncpy_s(newFilePath, newFilePathSize, outPath, newFilePathSize);
    if (ret != EOK) {
        return ARCHIVE_PARAM_ERROR;
    }
    newFilePath[newFilePathSize - 1] = '\0';

    if (IsFileExists(newFilePath) == ARCHIVE_OK) {
        ret = GenerateNewFilename(outPath, newFilePath, newFilePathSize);
        RETURN_IF_FAIL(ret);
    }
    return ARCHIVE_OK;
}

LOCAL int ZipReaderUpdateProgress(struct ZipReaderExtractContext *context, struct ReadResource *resource, bool force)
{
    struct ZipReader *reader = context->reader;
    if (reader->progressTotal == 0) {
        return OH_ARCHIVE_PROGRESS_CONTINUE;
    }

    if (reader->progressHandlerWithData) {
        size_t totalIn = StreamGetTotalIn(resource->compressStream);
        if (totalIn >  context->entryInfo.compressedSize) {
            totalIn = context->entryInfo.compressedSize;
        }
        totalIn += sizeof(struct LocalFileHeader) + context->localFileHeader.fileNameLength +
            context->localFileHeader.extraFieldLength;
        size_t currentUpdate = (context->progressPrevious + totalIn) - context->progressUpdated;
        if ((currentUpdate > PROGRESS_UPDATE_INTERVAL) || force) {
            context->progressUpdated = context->progressPrevious + totalIn;
            int progress = context->progressUpdated * PROGRESS_PERCENT / reader->progressTotal;
            // progress must be less than or equal to 100%
            return reader->progressHandlerWithData(progress <= 100 ? progress : 100, reader->progressHandlerData);
        }
    }

    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

LOCAL int  ZipReaderSaveFile(struct ZipReaderExtractContext *context, struct ReadResource *resource,
    const char *newFilePath)
{
    int ret;
    int64_t read;
    ret = ZipReaderUpdateProgress(context, resource, true);
    if (ret == OH_ARCHIVE_PROGRESS_CANCEL) {
        ZipReaderCloseEntry(context, resource);
        // 解压取消时删除当前未解压完的文件
        DeleteTempFileWithRootPath(context->outDir, newFilePath);
        return ARCHIVE_CANCEL_ERROR;
    }
    do {
        read = ZipReaderReadData(context, resource, resource->buffer, sizeof(resource->buffer));
        if (read < 0) {
            ZipReaderCloseEntry(context, resource);
            return ARCHIVE_READ_ERROR;
        }
        if (read > 0) {
            ret = ZipReaderWriteData(resource, resource->buffer, read);
            if (ret != ARCHIVE_OK) {
                ZipReaderCloseEntry(context, resource);
                return ret;
            }
        }
        ret = ZipReaderUpdateProgress(context, resource, false);
        if (ret == OH_ARCHIVE_PROGRESS_CANCEL) {
            ZipReaderCloseEntry(context, resource);
            // 解压取消时删除当前未解压完的文件
            DeleteTempFileWithRootPath(context->outDir, newFilePath);
            return ARCHIVE_CANCEL_ERROR;
        }
    } while (read > 0);
    ZipReaderUpdateProgress(context, resource, true);
    ret = ZipReaderVerifyEntryCrc(context, resource);
    ZipReaderCloseEntry(context, resource);
    return ret;
}

LOCAL int ZipReaderCreateSymlink(struct ZipReaderExtractContext *context, struct ReadResource *resource,
    const char *newFilePath)
{
    if (GETV(context->entryInfo.uncompressedSize) >= UINT16_MAX) {
        return ARCHIVE_FORMAT_ERROR;
    }
    int ret;
    int64_t read;
    read = ZipReaderReadData(context, resource, resource->buffer, sizeof(resource->buffer));
    if (read < 0) {
        ZipReaderCloseEntry(context, resource);
        return ARCHIVE_READ_ERROR;
    }
    resource->buffer[GETV(context->entryInfo.uncompressedSize)] = '\0';
    if (IsSymlinkExists(newFilePath) == ARCHIVE_OK) {
        unlink(newFilePath);
    }
    ret = CreateSymlink((const char *)resource->buffer, newFilePath);
    ZipReaderCloseEntry(context, resource);
    return ret;
}

LOCAL int ZipReaderSetFileTime(struct ZipReaderExtractContext *context, const char *path)
{
    struct utimbuf ut;
    ut.modtime = context->entryInfo.modifiedTime;
    time_t currentTime;
    (void)time(&currentTime);
    ut.actime = currentTime;
    if (utime(path, &ut) != 0) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    return ARCHIVE_OK;
}

LOCAL int ZipReaderSetFileAttribs(struct ZipReaderExtractContext *context, const char *path)
{
    int ret;
    uint32_t attribs;
    ret = ZipConvertAttrib(ZIP_HOST_SYSTEM(GETV(context->centralDirHeader.versionMadeBy)),
        GETV(context->centralDirHeader.externalFileAttr),
        &attribs);
    RETURN_IF_FAIL(ret);
    if (chmod(path, (mode_t)attribs) == -1) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    return ARCHIVE_OK;
}

LOCAL int ZipReaderExtractEntry(struct ZipReader *reader, const char *outPath)
{
    int ret;
    if (ZipReaderEntryIsDirectory(&reader->context) && !ZipReaderEntryIsSymlink(&reader->context.centralDirHeader)) {
        return CreateDirectory(outPath, NULL);
    }
    char newFilePath[ZIP_FILE_NAME_LEN_MAX];
    ret = ZipReaderAutoRenamePath(
        reader, reader->context.entryInfo.entryName, outPath, newFilePath, ZIP_FILE_NAME_LEN_MAX);
    RETURN_IF_FAIL(ret);
    ret = CreateParentDirectory(newFilePath, NULL);
    RETURN_IF_FAIL(ret);
    ret = ZipReaderOpenEntry(&reader->context, &reader->resource, newFilePath);
    RETURN_IF_FAIL(ret);
    if (ZipReaderEntryIsSymlink(&reader->context.centralDirHeader)) {
        return ZipReaderCreateSymlink(&reader->context, &reader->resource, newFilePath);
    } else {
        ret = ZipReaderSaveFile(&reader->context, &reader->resource, newFilePath);
    }
    RETURN_IF_FAIL(ret);

    // Set the time of the file
    ZipReaderSetFileTime(&reader->context, newFilePath);
    // Set the attributes of the file
    ZipReaderSetFileAttribs(&reader->context, newFilePath);
    return ret;
}

LOCAL int ZipReaderExtractAllFiles(HmArchiveReadInfo *archive, const char *outDir)
{
    if (archive == NULL || outDir == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret;
    char outPath[ZIP_FILE_NAME_LEN_MAX];
    struct ZipReader *reader = (struct ZipReader *)archive->fmtInfo;
    if (reader == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    reader->progressHandlerWithData = archive->progressHandlerWithData;
    reader->progressHandlerData = archive->progressHandlerData;

    reader->progressTotal = reader->offsetOfCentralDir;
    reader->context.progressPrevious = 0;
    reader->context.progressUpdated = 0;
    ret = ZipReaderGotoFirstCentralDirHeader(reader);
    RETURN_IF_FAIL(ret);
    do {
        reader->context.outDir = outDir;
        ret = GetOutputFilePath(reader->context.entryInfo.entryName, outDir, outPath, ZIP_FILE_NAME_LEN_MAX);
        RETURN_IF_FAIL(ret);
        ret = ZipReaderExtractEntry(reader, outPath);
        RETURN_IF_FAIL(ret);
        reader->context.progressPrevious += reader->context.entryInfo.compressedSize + sizeof(struct LocalFileHeader) +
            reader->context.localFileHeader.fileNameLength + reader->context.localFileHeader.extraFieldLength;
        ret = ZipReaderGotoNextCentralDirHeader(reader);
    } while (ret == ARCHIVE_OK);
    if (ret == ARCHIVE_END_OF_LIST) {
        return ARCHIVE_OK;
    }
    return ret;
}

LOCAL int ZipReaderClose(HmArchiveReadInfo *archive)
{
    if (archive == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    struct ZipReader *reader = (struct ZipReader *)archive->fmtInfo;
    if (reader == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    if (reader->resource.baseStream != NULL) {
        StreamClose(reader->resource.baseStream);
        StreamDestroy(&reader->resource.baseStream);
        reader->resource.baseStream = NULL;
    }
    ZipReaderDestroy(&reader);

    return ARCHIVE_OK;
}

const FmtReaderOps g_ZipReaderFmtOps = {
    .open = ZipReaderOpenFile,
    .extract = ZipReaderExtractAllFiles,
    .close = ZipReaderClose,
};