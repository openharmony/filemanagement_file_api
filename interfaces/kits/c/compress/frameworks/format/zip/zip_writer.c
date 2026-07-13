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
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "errorcode.h"
#include <stdatomic.h>
#include <unistd.h>
#include "zip_util.h"
#include "zip_writer_impl.h"
#include "zip_handler_common.h"
#include "zip_format.h"
#include "archive_macros.h"
#include "archive_inner.h"
#include "stream.h"
#include "compress/stream_deflate.h"
#include "raw/stream_raw.h"
#include "memory/stream_mem.h"
#include "file/stream_file.h"
#include "zlib.h"
#include "securec.h"
#include "oh_archive_writer.h"
#include "resource/container/dir_item_queue.h"

#define ARCHIVE_APPEND_STATUS_CREATE (0)
#define ZIP_VERSION_20 (20)
#define ZIP_VERSION_45 (45)

#define ARCHIVE_OPEN_MODE_WRITE (0x02)
#define ARCHIVE_WZAES_COMPRESS_METHOD (99)
#define COMPRESS_LEVEL_1 (1)
#define COMPRESS_LEVEL_9 (9)
#define DEFAULT_CD_COMMENT_LENGTH (0)
#define ZLIB_DEFLATE_DEF_MEM_LEVEL (8)
#define ZLIB_DEFLATE_STRATEGY (0)
#define ZIP_FLAG_UTF8 (1 << 11)
#define NEED_ZIP64 (1)
#define NO_NEED_ZIP64 (0)
#define IS_SYMLINK (1)
#define NOT_SYMLINK (0)
#define COMPRESS_OVER_PROCESS (100)

struct ZipWriterEntryInfo {
    uint16_t versionMadeBy;
    uint16_t flag;
    uint16_t compressionMethod;
    uint32_t crc;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
    uint16_t filenameSize;
    uint32_t diskNumber;
    uint64_t localHeaderOffset;
    uint16_t internalFa;
    uint32_t externalFa;
    const char *filename;
    uint32_t dosModDateTime;
    uint16_t zip64;
    uint64_t zip64extraInLocalHeaderPos;
};

struct ZipWriter {
    struct Stream *baseStream;
    struct Stream *cdMemStream;
    OH_Archive_ProgressHandlerWithData progressHandler;
    void *progressHandlerUserData;
    uint64_t progressTotal;
    uint64_t progressUpdated;
    uint16_t globalCommentLength;
    char *globalComment;
    uint64_t offsetOfGlobalComment;

    uint32_t numberOfDiskWithCD;
    uint16_t versionMadeBy;

    uint64_t offsetOfEOCD;
    uint64_t offsetOfCentralDir;
    uint64_t sizeOfCentralDir;
    int8_t isZip64;
    uint64_t totalNumberOfEntries;
    int mode;
    int isOpened;
    char outfile[ZIP_FILE_NAME_LEN_MAX + 1];
};

typedef enum {
    FILE_TYPE_FILE,
    FILE_TYPE_EMPTY_DIR
} FileType;

typedef struct FilePathNode {
    char *filePath;
    char *saveName;
} FilePathNode;

typedef FilePathNode *FilePathNodePtr;

struct ZipWriterTaskParam {
    HmArchiveWriteInfo *archive;
    struct ZipWriter *writer;
    struct Stream *compressStream;
    struct Stream *extractStream;
    struct LocalFileHeader localFileHeader;
    struct ZipWriterEntryInfo *entryInfo;
    FileType fileType;
    uint8_t buffer[ZIP_BUF_INTERNAL];
    FilePathNode *fileNode;
};

static uint32_t ZipConvertAppendToStreamMode(void)
{
    return OPEN_MODE_WRITE | OPEN_MODE_CREATE;
}

static struct ZipWriter *ZipWriterCreate(void)
{
    struct ZipWriter *writer = (struct ZipWriter *)calloc(1, sizeof(struct ZipWriter));
    if (writer) {
        writer->baseStream = NULL;
        writer->cdMemStream = NULL;
        writer->globalComment = NULL;
    }
    return writer;
}

static void ZipWriterOpenFail(struct ZipWriter *writer)
{
    ASSERT(writer);
    if (writer->baseStream) {
        StreamClose(writer->baseStream);
        StreamDestroy(&writer->baseStream);
        writer->baseStream = NULL;
    }
    if (writer->cdMemStream) {
        StreamClose(writer->cdMemStream);
        StreamDestroy(&writer->cdMemStream);
        writer->cdMemStream = NULL;
    }
    if (writer->globalComment) {
        free(writer->globalComment);
        writer->globalComment = NULL;
    }
}

static int ZipWriterOpenFileOpenStream(struct ZipWriter *writer, int mode)
{
    int ret = ARCHIVE_OK;
    ret = StreamOpen(writer->baseStream, mode);
    if (ret == ARCHIVE_OK) {
        writer->cdMemStream = MemStreamCreate();
        if (writer->cdMemStream != NULL) {
            ret = StreamOpen(writer->cdMemStream, ARCHIVE_OPEN_MODE_WRITE);
        } else {
            ret = ARCHIVE_MEM_ERROR;
        }
    }
    return ret;
}

static int ZipWriterOpenOutputFile(HmArchiveWriteInfo *archive)
{
    if (archive == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    struct ZipWriter *writer = (struct ZipWriter *)archive->fmtInfo;
    int ret = ARCHIVE_OK;
    uint32_t mode = ZipConvertAppendToStreamMode();
    ret = ZipWriterOpenFileOpenStream(writer, (int)mode);
    if (ret != ARCHIVE_OK) {
        ZipWriterOpenFail(writer);
    }
    return ret;
}

static int ZipWriterOpenFileCreateWriteStream(struct ZipWriter *writer, const char *outfile)
{
    struct Stream *stream = FileStreamCreate(outfile);
    if (stream == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    writer->baseStream = stream;
    if (EOK != strncpy_s(writer->outfile, ZIP_FILE_NAME_LEN_MAX, outfile, strlen(outfile))) {
        writer->outfile[0] = '\0';
        return ARCHIVE_INTERNAL_ERROR;
    }
    writer->outfile[ZIP_FILE_NAME_LEN_MAX] = '\0';
    return ARCHIVE_OK;
}

static int ZipWriterOpenFile(HmArchiveWriteInfo *archive, const char *outfile)
{
    int ret = ARCHIVE_OK;
    if (archive == NULL || outfile == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    struct ZipWriter *writer = ZipWriterCreate();
    if (writer == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    archive->fmtInfo = writer;

    ret = ZipWriterOpenFileCreateWriteStream(writer, outfile);
    if (ret != ARCHIVE_OK) {
        ZipWriterOpenFail(writer);
        free(archive->fmtInfo);
        archive->fmtInfo = NULL;
    }
    return ret;
}

static int ZipWriterUpdateProgress(struct ZipWriter *writer, size_t len, bool isCompressOver)
{
    if (writer->progressHandler == NULL) {
        return OH_ARCHIVE_PROGRESS_CONTINUE;
    }

    if (writer->progressTotal == 0 || isCompressOver) {
        int ratio = 0;
        if (isCompressOver) {
            ratio = COMPRESS_OVER_PROCESS;
        }
        if (writer->progressHandler) {
            return writer->progressHandler(ratio, writer->progressHandlerUserData);
        }
    }
    writer->progressUpdated = writer->progressUpdated + len;
    int ratio = (int)(writer->progressUpdated * PROGRESS_PERCENT / writer->progressTotal);
    return writer->progressHandler(ratio, writer->progressHandlerUserData);
}

static int ZipWriterOpenInitCompressStream(struct ZipWriterTaskParam *writerTask)
{
    switch (writerTask->entryInfo->compressionMethod) {
    case OH_ARCHIVE_NO_COMPRESSION:
        writerTask->compressStream = RawStreamCreate();
        break;
    case OH_ARCHIVE_COMPRESS_DEFLATE:
        writerTask->compressStream =
            DeflateStreamCreateSpec(Z_DEFLATED, writerTask->archive->level,
                                    -MAX_WBITS, ZLIB_DEFLATE_DEF_MEM_LEVEL, ZLIB_DEFLATE_STRATEGY);
        break;
    default:
        writerTask->compressStream = NULL;
        return ARCHIVE_SUPPORT_ERROR;
    }
    if (writerTask->compressStream == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    StreamSetBase(writerTask->compressStream, writerTask->writer->baseStream);
    return StreamOpen(writerTask->compressStream, ARCHIVE_OPEN_MODE_WRITE);
}

static int ZipWriterOpenInitExtractStream(struct ZipWriterTaskParam *writerTask)
{
    if (writerTask->fileType == FILE_TYPE_EMPTY_DIR) {
        writerTask->extractStream = NULL;
        return ARCHIVE_OK;
    } else {
        writerTask->extractStream = FileStreamCreate(writerTask->fileNode->filePath);
        if (writerTask->extractStream == NULL) {
            return ARCHIVE_MEM_ERROR;
        }
        return StreamOpen(writerTask->extractStream, ARCHIVE_OPEN_MODE_READ);
    }
}

#define ZIP_EXTRAFIELD_ZIP64_HEADER 0x0001
#define ZIP_LOCAL_EXTRAFILED_ZIP64_LENGTH 16
#define ZIP_CD_EXTRA_ZIP64_LENGTH 24
#define ZIP_END_OF_CD64_SIZE 44

static int ZipWriterDataDesiptorZip64LocalHeader(struct Stream *stream, struct ZipWriterEntryInfo *entry)
{
    entry->zip64extraInLocalHeaderPos = (uint64_t)StreamTell(stream);
    struct LocalExtraDataZIP64 localExtraDataZIP64;
    localExtraDataZIP64.signature = SETV(ZIP_EXTRAFIELD_ZIP64_HEADER);
    localExtraDataZIP64.extraFieldLength = SETV(ZIP_LOCAL_EXTRAFILED_ZIP64_LENGTH);
    localExtraDataZIP64.compressedSize = SETV(entry->compressedSize);
    localExtraDataZIP64.uncompressedSize = SETV(entry->uncompressedSize);
    int64_t writeRet = StreamWrite(stream, &localExtraDataZIP64, sizeof(struct LocalExtraDataZIP64));
    if (writeRet != (int64_t)sizeof(struct LocalExtraDataZIP64)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
};

#define ZIP64_COMPRESS_SIZE 8
#define ZIP64_UNCOMPRESS_SIZE 8
#define ZIP64_LOCALHEADEROFFSET_SIZE 8
static int ZipWriteExtraFieldZip64Cd(struct Stream *stream, struct ZipWriterEntryInfo *entry,
                                     uint16_t zip64ExtraFieldDataSize)
{
    int ret = ARCHIVE_OK;
    if (zip64ExtraFieldDataSize == 0) {
        return ARCHIVE_OK;
    }
    uint16_t sizeCheck = 0;
    struct CdExtraDataZIP64 cdExtraDataZIP64;
    cdExtraDataZIP64.signature = SETV(ZIP_EXTRAFIELD_ZIP64_HEADER);
    cdExtraDataZIP64.extraFieldLength = SETV(zip64ExtraFieldDataSize);
    int64_t writeRet = StreamWrite(stream, &cdExtraDataZIP64, sizeof(struct CdExtraDataZIP64));
    if (writeRet != (int64_t)sizeof(struct CdExtraDataZIP64)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    if (entry->uncompressedSize > UINT32_MAX) {
        sizeCheck += ZIP64_UNCOMPRESS_SIZE;
        ret = StreamWriteUint64(stream, (uint64_t)entry->uncompressedSize);
        RETURN_IF_FAIL(ret);
    }
    if (entry->compressedSize > UINT32_MAX) {
        sizeCheck += ZIP64_COMPRESS_SIZE;
        ret = StreamWriteUint64(stream, (uint64_t)entry->compressedSize);
        RETURN_IF_FAIL(ret);
    }
    if (entry->localHeaderOffset > UINT32_MAX) {
        sizeCheck += ZIP64_LOCALHEADEROFFSET_SIZE;
        ret = StreamWriteUint64(stream, (uint64_t)entry->localHeaderOffset);
        RETURN_IF_FAIL(ret);
    }
    if (sizeCheck != zip64ExtraFieldDataSize) {
        ret = ARCHIVE_WRITE_ERROR;
    }
    return ret;
}

#define ZIP_4GB_MAX_UNCOMPRESSED (0xff000000)
static inline int NeedZip64(uint64_t size)
{
    if (size > ZIP_4GB_MAX_UNCOMPRESSED) {
        return NEED_ZIP64;
    } else {
        return NO_NEED_ZIP64;
    }
}

static int ZipEntryWriteLocalHeader(struct Stream *stream, struct ZipWriterEntryInfo *entry)
{
    struct LocalFileHeader partLocalFileHeader;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
    uint16_t versionNeeded = ZIP_VERSION_20;
    uint32_t extrafieldSizeTotal = 0;
    if (entry->zip64 == NEED_ZIP64) {
        versionNeeded = ZIP_VERSION_45;
        extrafieldSizeTotal += sizeof(struct LocalExtraDataZIP64);
    }
    compressedSize = entry->compressedSize > UINT32_MAX ? UINT32_MAX : entry->compressedSize;
    uncompressedSize = entry->uncompressedSize > UINT32_MAX ? UINT32_MAX : entry->uncompressedSize;
    partLocalFileHeader.signature = SETV(ZIP_MAGIC_LOCALHEADER);
    partLocalFileHeader.versionNeeded = SETV(versionNeeded);
    partLocalFileHeader.flag = SETV(entry->flag);
    partLocalFileHeader.compressionMethod = SETV(entry->compressionMethod);

    if (entry->flag & ZIP_FLAG_ENCRYPTED) {
        partLocalFileHeader.compressionMethod = SETV((uint16_t)ARCHIVE_WZAES_COMPRESS_METHOD);
    }
    partLocalFileHeader.dosModDateTime = SETV(entry->dosModDateTime);
    partLocalFileHeader.crc32 = SETV((uint32_t)entry->crc);
    partLocalFileHeader.compressedSize = SETV((uint32_t)compressedSize);
    partLocalFileHeader.uncompressedSize = SETV((uint32_t)uncompressedSize);
    partLocalFileHeader.fileNameLength = SETV(entry->filenameSize);
    partLocalFileHeader.extraFieldLength = SETV((uint16_t)extrafieldSizeTotal);
    int64_t writeRet = StreamWrite(stream, &partLocalFileHeader, sizeof(struct LocalFileHeader));
    if (writeRet != (int64_t)sizeof(struct LocalFileHeader)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }

    int ret = ZipWriteFileName(stream, entry->filename, entry->filenameSize);
    RETURN_IF_FAIL(ret);

    if (entry->zip64) {
        return ZipWriterDataDesiptorZip64LocalHeader(stream, entry);
    }

    return ARCHIVE_OK;
}

static uint16_t GetCdExtraFieldZip64DataSize(struct ZipWriterEntryInfo *entry)
{
    uint32_t zip64ExtraFieldDataSize = 0;
    if (entry->uncompressedSize > UINT32_MAX) {
        zip64ExtraFieldDataSize += ZIP64_UNCOMPRESS_SIZE;
    }

    if (entry->compressedSize > UINT32_MAX) {
        zip64ExtraFieldDataSize += ZIP64_COMPRESS_SIZE;
    }

    if (entry->localHeaderOffset > UINT32_MAX) {
        zip64ExtraFieldDataSize += ZIP64_LOCALHEADEROFFSET_SIZE;
    }

    return zip64ExtraFieldDataSize;
}

static int ZipWritePartCentralDirHeader(struct Stream *stream, struct ZipWriterEntryInfo *entry,
                                        uint16_t zip64ExtraFieldDataSize)
{
    uint64_t compressedSize = entry->compressedSize > UINT32_MAX ? UINT32_MAX : entry->compressedSize;
    uint64_t uncompressedSize = entry->uncompressedSize > UINT32_MAX ? UINT32_MAX : entry->uncompressedSize;
    uint64_t relativeOffsetOfLocalHeader = entry->localHeaderOffset > UINT32_MAX ?
                                            UINT32_MAX : entry->localHeaderOffset;
    uint16_t cdVersionNeeded = ZIP_VERSION_20;
    uint32_t extrafieldSizeTotal = 0;
    if (zip64ExtraFieldDataSize > 0) {
        extrafieldSizeTotal = extrafieldSizeTotal + sizeof(struct CdExtraDataZIP64) + zip64ExtraFieldDataSize;
        cdVersionNeeded = ZIP_VERSION_45;
    }
    struct CentralDirHeader partCentralDirHeader;

    partCentralDirHeader.signature = SETV(ZIP_MAGIC_CENTRALHEADER);
    partCentralDirHeader.versionMadeBy = SETV(entry->versionMadeBy);
    partCentralDirHeader.versionNeeded = SETV(cdVersionNeeded);
    partCentralDirHeader.flag = SETV(entry->flag);
    partCentralDirHeader.compressionMethod = SETV(entry->compressionMethod);

    if (entry->flag & ZIP_FLAG_ENCRYPTED) {
        partCentralDirHeader.compressionMethod = SETV((uint16_t)ARCHIVE_WZAES_COMPRESS_METHOD);
    }
    partCentralDirHeader.dosModDateTime = SETV(entry->dosModDateTime);
    partCentralDirHeader.crc32 = SETV(entry->crc);
    partCentralDirHeader.compressedSize = SETV(compressedSize);
    partCentralDirHeader.uncompressedSize = SETV(uncompressedSize);
    partCentralDirHeader.fileNameLength = SETV(entry->filenameSize);
    partCentralDirHeader.extraFieldLength = SETV((uint16_t)extrafieldSizeTotal);

    partCentralDirHeader.fileCommentLength = SETV(DEFAULT_CD_COMMENT_LENGTH);
    partCentralDirHeader.diskNumberStart = SETV(entry->diskNumber);
    partCentralDirHeader.internalFileAttr = SETV(entry->internalFa);
    partCentralDirHeader.externalFileAttr = SETV(entry->externalFa);
    partCentralDirHeader.relativeOffsetOfLocalHeader = SETV(relativeOffsetOfLocalHeader);

    int64_t writeRet = StreamWrite(stream, &partCentralDirHeader, sizeof(struct CentralDirHeader));
    if (writeRet != (int64_t)sizeof(struct CentralDirHeader)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipEntryWriteCdHeader(struct Stream *stream, struct ZipWriterEntryInfo *entry)
{
    int ret;
    uint16_t zip64ExtraFieldDataSize = GetCdExtraFieldZip64DataSize(entry);
    ret = ZipWritePartCentralDirHeader(stream, entry, zip64ExtraFieldDataSize);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    ret = ZipWriteFileName(stream, entry->filename, entry->filenameSize);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    if (zip64ExtraFieldDataSize > 0) {
        ret = ZipWriteExtraFieldZip64Cd(stream, entry, zip64ExtraFieldDataSize);
    }
    return ret;
};

static int ZipEntryUpdateLocalHeader(struct Stream *stream, struct ZipWriterEntryInfo *entry, bool inCache)
{
    int ret;
    int64_t currentPos = StreamTell(stream);
    int64_t crcPos = (int64_t)entry->localHeaderOffset + CRC_POS_IN_LOCAL_HEADER;
    if (inCache) {
        crcPos = CRC_POS_IN_LOCAL_HEADER;
    }
    ret = StreamSeek(stream, crcPos, ARCHIVE_SEEK_SET);
    RETURN_IF_FAIL(ret);
    ret = StreamWriteUint32(stream, (uint32_t)entry->crc);
    RETURN_IF_FAIL(ret);
    if (entry->compressedSize > UINT32_MAX || entry->uncompressedSize > UINT32_MAX) {
        if (!entry->zip64) {
            StreamSeek(stream, currentPos, ARCHIVE_SEEK_SET);
            return ARCHIVE_PARAM_ERROR;
        }
    }

    uint64_t compressedSize = entry->compressedSize > UINT32_MAX ? UINT32_MAX : entry->compressedSize;
    uint64_t uncompressedSize = entry->uncompressedSize > UINT32_MAX ? UINT32_MAX : entry->uncompressedSize;
    ret = StreamWriteUint32(stream, (uint32_t)compressedSize);
    RETURN_IF_FAIL(ret);
    ret = StreamWriteUint32(stream, (uint32_t)uncompressedSize);
    RETURN_IF_FAIL(ret);

    if (entry->zip64 && entry->zip64extraInLocalHeaderPos > 0) {
        ret = StreamSeek(stream, entry->zip64extraInLocalHeaderPos + EXTRA_FIELD_HEADER_BYTES, ARCHIVE_SEEK_SET);
        RETURN_IF_FAIL(ret);
        ret = StreamWriteUint64(stream, entry->uncompressedSize);
        RETURN_IF_FAIL(ret);
        ret = StreamWriteUint64(stream, entry->compressedSize);
        RETURN_IF_FAIL(ret);
    }

    ret = StreamSeek(stream, currentPos, ARCHIVE_SEEK_SET);
    return ret;
}

static void UpdateCompressedSize(struct ZipWriterTaskParam *writerTask)
{
    writerTask->entryInfo->compressedSize = StreamGetTotalOut(writerTask->compressStream);
    writerTask->entryInfo->uncompressedSize = StreamGetTotalIn(writerTask->compressStream);
}

static inline int ZipWriterCloseStreamCheckInput(struct ZipWriterTaskParam *writerTask)
{
    if (writerTask == NULL || writerTask->compressStream == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    if ((writerTask->fileType != FILE_TYPE_EMPTY_DIR) && (writerTask->extractStream == NULL)) {
        return ARCHIVE_PARAM_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipWriterCloseStream(struct ZipWriterTaskParam *writerTask, int needCalSize)
{
    int ret = ARCHIVE_OK;
    if (ZipWriterCloseStreamCheckInput(writerTask) != ARCHIVE_OK) {
        return ARCHIVE_PARAM_ERROR;
    }

    ret = StreamClose(writerTask->compressStream);
    RETURN_IF_FAIL(ret);

    if (needCalSize) {
        UpdateCompressedSize(writerTask);
    }

    ret = ZipEntryUpdateLocalHeader(writerTask->writer->baseStream, writerTask->entryInfo, 0);
    RETURN_IF_FAIL(ret);
    ret = ZipEntryWriteCdHeader(writerTask->writer->cdMemStream, writerTask->entryInfo);
    writerTask->writer->totalNumberOfEntries += 1;
    StreamDestroy(&writerTask->compressStream);
    return ret;
}

static int IsSymlink(const char *path)
{
    struct stat pathStat;
    memset_s(&pathStat, sizeof(pathStat), 0, sizeof(pathStat));

    if (lstat(path, &pathStat) != 0) {
        return NOT_SYMLINK;
    }

    if (S_ISLNK(pathStat.st_mode)) {
        return IS_SYMLINK;
    }
    return NOT_SYMLINK;
}

static int ReadSymlink(const char *path, char *targetPath, int32_t maxTargetPath)
{
    size_t length = 0;
    length = (size_t)readlink(path, targetPath, maxTargetPath - 1);
    if (length == (size_t)-1) {
        return ARCHIVE_EXIST_ERROR;
    }
    targetPath[length] = '\0';
    return ARCHIVE_OK;
}

static int ZipWriterAddData(struct ZipWriterTaskParam *writerTask)
{
    ASSERT(writerTask != NULL);
    size_t len = 0;
    if (IsSymlink(writerTask->fileNode->filePath) == IS_SYMLINK) {
        char linkName[ZIP_FILE_NAME_LEN_MAX];
        if (ReadSymlink(writerTask->fileNode->filePath, linkName, ZIP_FILE_NAME_LEN_MAX) != ARCHIVE_OK) {
            return ARCHIVE_PARAM_ERROR;
        }
        len = strlen(linkName);
        int writerRet = ARCHIVE_OK;
        if ((writerRet = StreamWrite(writerTask->compressStream, linkName, len)) != (int64_t)len) {
            return writerRet < 0 ? writerRet : ARCHIVE_WRITE_ERROR;
        }
        if (len > 0) {
            writerTask->entryInfo->crc = crc32(writerTask->entryInfo->crc, (const uint8_t *)linkName, len);
        }
        return ARCHIVE_OK;
    }
    int64_t readRet = ARCHIVE_OK;
    int64_t writeRet = ARCHIVE_OK;
    while ((readRet = StreamRead(writerTask->extractStream, writerTask->buffer, sizeof(writerTask->buffer))) > 0) {
        if ((writeRet = StreamWrite(writerTask->compressStream, writerTask->buffer, readRet)) != readRet) {
            return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
        }
        /* code */
        if (writeRet > 0) {
            writerTask->entryInfo->crc = crc32(writerTask->entryInfo->crc, writerTask->buffer, writeRet);
            int ret = ZipWriterUpdateProgress(writerTask->writer, writeRet, 0);
            if (ret == OH_ARCHIVE_PROGRESS_CANCEL) {
                return ARCHIVE_CANCEL_ERROR;
            }
        }
    }
    if (readRet < 0) {
        return readRet;
    }

    return ARCHIVE_OK;
}

static inline uint64_t GetFileSize(const char *infile)
{
    uint64_t size = 0;
    struct stat statbuf;
    if (stat(infile, &statbuf) == 0) {
        size = (uint64_t)statbuf.st_size;
    }
    return size;
}

#define VERSION_MADEBY_LOW_BITS 8
static inline uint16_t GetVersionMadeBy(HmArchiveWriteInfo *archive)
{
    return ZIP_VERSION_MADEBY;
}

#define EXTER_ATTRIB_LOW_BITS 16
static uint32_t GetFileExternalAttributes(const char *path)
{
    uint32_t attribute;
    uint32_t msdosAttribute;
    struct stat pathStat;
    memset_s(&pathStat, sizeof(pathStat), 0, sizeof(pathStat));
    if (stat(path, &pathStat) == -1) {
        attribute = 0;
    } else {
        attribute = pathStat.st_mode;
    }
    msdosAttribute = attribute >> EXTER_ATTRIB_LOW_BITS;
    attribute = msdosAttribute | (attribute << EXTER_ATTRIB_LOW_BITS);
    return attribute;
}

static uint32_t GetFileModfiyTime(const char *path)
{
    struct stat pathStat;
    uint32_t time = 0;
    memset_s(&pathStat, sizeof(pathStat), 0, sizeof(pathStat));
    if (lstat(path, &pathStat) == 0) {
        time = ZipConvertTimeToDosDate(pathStat.st_mtime);
    } else {
        time = ZipGetCurrentDosModeDateTime();
    }
    return time;
}

static uint16_t ZipGetFlag(uint16_t compressionMethod, int compressLevel)
{
    uint16_t flag = 0;
    if (compressionMethod == OH_ARCHIVE_COMPRESS_DEFLATE) {
        if (compressLevel == COMPRESS_LEVEL_9) {
            flag |= ZIP_FLAG_DEFLATE_MAX;
        } else if (compressLevel == COMPRESS_LEVEL_1) {
            flag |= ZIP_FLAG_DEFLATE_SUPER_FAST;
        }
    }
    return flag;
}

static void ZipWriterOpenInitEntryInfo(struct ZipWriterTaskParam *writerTask)
{
    struct ZipWriterEntryInfo *entryInfo = writerTask->entryInfo;
    entryInfo->compressionMethod = (uint16_t)writerTask->archive->method;
    entryInfo->versionMadeBy = GetVersionMadeBy(writerTask->archive);
    entryInfo->dosModDateTime = GetFileModfiyTime(writerTask->fileNode->filePath);
    entryInfo->internalFa = 0;
    entryInfo->externalFa = GetFileExternalAttributes(writerTask->fileNode->filePath);
    entryInfo->localHeaderOffset = (uint64_t)StreamTell(writerTask->writer->baseStream);
    entryInfo->filename = writerTask->fileNode->saveName;
    entryInfo->filenameSize = entryInfo->filename == NULL ? 0 : (uint16_t)strlen(entryInfo->filename);
    entryInfo->compressedSize = 0;
    entryInfo->uncompressedSize = GetFileSize(writerTask->fileNode->filePath);
    entryInfo->crc = 0;
    entryInfo->zip64 = NeedZip64(entryInfo->uncompressedSize);
    entryInfo->diskNumber = 0;
    entryInfo->flag = ZipGetFlag(writerTask->archive->method, writerTask->archive->level);
    if (writerTask->fileType == FILE_TYPE_EMPTY_DIR) {
        entryInfo->compressionMethod = OH_ARCHIVE_NO_COMPRESSION;
        entryInfo->uncompressedSize = 0;
    }
    if (IsUtf8Encoded((unsigned char *)entryInfo->filename, false)) {
        entryInfo->flag = entryInfo->flag | ZIP_FLAG_UTF8;
    }
}

static int ZipWriterEntryOpen(struct ZipWriterTaskParam *writerTask)
{
    writerTask->entryInfo = (struct ZipWriterEntryInfo *)calloc(1, sizeof(struct ZipWriterEntryInfo));
    if (writerTask->entryInfo == NULL) {
        return ARCHIVE_MEM_ERROR;
    }
    if ((writerTask->archive->method != OH_ARCHIVE_NO_COMPRESSION) &&
        (writerTask->archive->method != OH_ARCHIVE_COMPRESS_DEFLATE)) {
        return ARCHIVE_PARAM_ERROR;
    }

    ZipWriterOpenInitEntryInfo(writerTask);

    if (strlen(writerTask->entryInfo->filename) == 0 || strlen(writerTask->entryInfo->filename) > 0xffff) {
        return ARCHIVE_PARAM_ERROR;
    }

    int ret = ZipWriterOpenInitExtractStream(writerTask);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    ret = ZipEntryWriteLocalHeader(writerTask->writer->baseStream, writerTask->entryInfo);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    ret = ZipWriterOpenInitCompressStream(writerTask);
    return ret;
}

static int32_t IsDir(const char *path)
{
    struct stat pathStat;
    memset_s(&pathStat, sizeof(pathStat), 0, sizeof(pathStat));
    if (stat(path, &pathStat) == 0) {
        if (S_ISDIR(pathStat.st_mode)) {
            return 1;
        }
    }
    return 0;
}

#define SLASH_END_SIZE 2
static int32_t PathAppendSlash(char *path, int32_t maxPath, char slash)
{
    int32_t pathLen = (int32_t)strlen(path);
    if ((pathLen + SLASH_END_SIZE) >= maxPath) {
        return ARCHIVE_BUF_ERROR;
    }
    if (path[pathLen - 1] != slash) {
        path[pathLen] = slash;
        path[pathLen + 1] = 0;
    }
    return ARCHIVE_OK;
}
#define ARCHIVE_PATH_SLASH_UNIX '/'
#define ARCHIVE_PATH_SLASH_WINDOWS '\\'

#define ARCHIVE_PATH_SLASH_PLATFORM ARCHIVE_PATH_SLASH_UNIX

static int32_t PathCombine(char *path, const char *join, int32_t maxPath)
{
    int32_t pathLen = 0;
    if (path == NULL || join == NULL || maxPath <= 0) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret = ARCHIVE_OK;
    pathLen = (int32_t)strlen(path);
    if (pathLen == 0) {
        if (strncpy_s(path, maxPath, join, strlen(join)) == EOK) {
            path[maxPath - 1] = 0;
        } else {
            ret = ARCHIVE_INTERNAL_ERROR;
        }
    } else {
        ret = PathAppendSlash(path, maxPath, ARCHIVE_PATH_SLASH_PLATFORM);
        if (ret != ARCHIVE_OK) {
            return ret;
        }
        pathLen = (int32_t)strlen(path);
        if (maxPath > pathLen) {
            if (strncat_s(path, maxPath, join, strlen(join)) != EOK) {
                ret = ARCHIVE_INTERNAL_ERROR;
            }
        } else {
            ret = ARCHIVE_NAME_TOO_LONG_ERROR;
        }
    }
    return ret;
}

static void ZipWriterDeleteStream(struct ZipWriterTaskParam *writerTask)
{
    if (writerTask->compressStream) {
        StreamClose(writerTask->compressStream);
        StreamDestroy(&writerTask->compressStream);
    }
    if (writerTask->extractStream) {
        StreamClose(writerTask->extractStream);
        StreamDestroy(&writerTask->extractStream);
    }
    if (writerTask->entryInfo) {
        free(writerTask->entryInfo);
        writerTask->entryInfo = NULL;
    }
}

static int32_t ZipWriterAddOneFile(HmArchiveWriteInfo *archive, struct ZipWriter *writer, FilePathNodePtr nodePtr)
{
    struct ZipWriterTaskParam param;
    memset_s(&param, sizeof(param), 0, sizeof(param));
    param.archive = archive;
    param.writer = writer;
    param.fileType = FILE_TYPE_FILE;
    param.fileNode = nodePtr;

    int ret = ZipWriterEntryOpen(&param);
    if (ret == ARCHIVE_OK) {
        ret = ZipWriterAddData(&param);
    } else if (ret == ARCHIVE_OPEN_ERROR) {
        if (access(nodePtr->filePath, F_OK) != 0) {
            return ARCHIVE_OK;
        }
    }
    if (ret == ARCHIVE_OK) {
        ret = ZipWriterCloseStream(&param, 1);
    }

    ZipWriterDeleteStream(&param);
    return ret;
}

static int IsValidFolderEnd(const char *dir)
{
    if (dir == NULL || strlen(dir) == 0) {
        return 0;
    }
    if (strcmp(dir, ".") == 0 || strcmp(dir, "..") == 0) {
        return 0;
    }
    return 1;
}

static int32_t TraversalDir(DirItemQueue *dirItemQueue, const char *currentDir)
{
    int ret = ARCHIVE_OK;
    DIR *dir = opendir(currentDir);
    if (dir == NULL) {
        return ARCHIVE_OK;
    }
    struct dirent *entry = NULL;
    char fullPath[ZIP_FILE_NAME_LEN_MAX] = {0};
    while (ret == ARCHIVE_OK) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        fullPath[0] = 0;
        ret = PathCombine(fullPath, currentDir, ZIP_FILE_NAME_LEN_MAX);
        if (ret == ARCHIVE_OK) {
            ret = PathCombine(fullPath, entry->d_name, ZIP_FILE_NAME_LEN_MAX);
        }
        if (ret == ARCHIVE_OK) {
            ret = PushDirItemQueueNode(dirItemQueue, fullPath);
        }
    }
    closedir(dir);
    return ret;
}

static int GetDirSize(const char *infile, uint64_t *totalSize)
{
    int ret = ARCHIVE_OK;
    DirItemQueue dirItemQueue;
    InitDirItemQueue(&dirItemQueue);
    PushDirItemQueueNode(&dirItemQueue, infile);
    while ((ret == ARCHIVE_OK) && !IsDirItemQueueEmpty(&dirItemQueue)) {
        char *path = PopDirItemQueueNode(&dirItemQueue);
        if (IsDir(path)) {
            if (IsValidFolderEnd(path)) {
                ret = TraversalDir(&dirItemQueue, path);
            }
        } else {
            struct stat statbuf;
            if (stat(path, &statbuf) == 0) {
                uint64_t size = (uint64_t)statbuf.st_size;
                (*totalSize) += size;
            }
        }
        free(path);
    }
    FreeDirItemQueue(&dirItemQueue);
    return ret;
}

static void GetprefixOfInput(const char *input, char *prefix)
{
    int len = (int)strlen(input);
    int lastSlashIndex = -1;
    for (int i = len - 2; i >= 0; --i) {
        if (input[i] == ARCHIVE_PATH_SLASH_PLATFORM) {
            lastSlashIndex = i;
            break;
        }
    }

    if (len > ZIP_FILE_NAME_LEN_MAX || lastSlashIndex < 0) {
        prefix[0] = '\0';
        if (strcmp(input, ".") == 0) {
            if (EOK != strncpy_s(prefix, ZIP_FILE_NAME_LEN_MAX, "./", strlen("./"))) {
                prefix[0] = '\0';
                return;
            }
        } else if (strcmp(input, "..") == 0) {
            if (EOK != strncpy_s(prefix, ZIP_FILE_NAME_LEN_MAX, "../", strlen("../"))) {
                prefix[0] = '\0';
                return;
            }
        }
        return;
    }

    if (EOK != strncpy_s(prefix, ZIP_FILE_NAME_LEN_MAX, input, lastSlashIndex + 1)) {
        prefix[0] = '\0';
        return;
    }
    prefix[ZIP_FILE_NAME_LEN_MAX] = '\0';
}

static int InitWriterProgress(HmArchiveWriteInfo *archive, struct ZipWriter *writer,
                              const char **infile, size_t fileNum)
{
    writer->progressHandler = archive->progressHandler;
    writer->progressHandlerUserData = archive->progressHandlerUserData;
    writer->progressUpdated = 0;
    uint64_t progressTotal = 0;
    for (size_t i = 0; i < fileNum; i++) {
        if (GetDirSize(infile[i], &progressTotal) != ARCHIVE_OK) {
            return ARCHIVE_INTERNAL_ERROR;
        }
    }
    writer->progressTotal = progressTotal;
    return ARCHIVE_OK;
}

static int GetSaveName(char *filePath, const char *inputDir, const char **saveName)
{
    size_t inputDirLen = strlen(inputDir);
    if (inputDirLen != 0 && strncmp(filePath, inputDir, inputDirLen) != 0) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    *saveName = filePath + inputDirLen;
    return ARCHIVE_OK;
}

int32_t ZipWriterAddEmptyDir(HmArchiveWriteInfo *archive, struct ZipWriter *writer, const char *inputDir, char *path)
{
    FilePathNode pathNode;
    pathNode.filePath = path;
    char newPath[ZIP_FILE_NAME_LEN_MAX + 1] = {0};
    if (EOK != strncpy_s(newPath, ZIP_FILE_NAME_LEN_MAX + 1, path, strlen(path))) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    int ret;
    ret = GetSaveName(newPath, inputDir, (const char **)&pathNode.saveName);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    ret = PathAppendSlash(newPath, ZIP_FILE_NAME_LEN_MAX, ARCHIVE_PATH_SLASH_PLATFORM);
    if (ret != ARCHIVE_OK) {
        return ret;
    }
    struct ZipWriterTaskParam param;
    memset_s(&param, sizeof(param), 0, sizeof(param));
    param.archive = archive;
    param.writer = writer;
    param.fileType = FILE_TYPE_EMPTY_DIR;
    param.fileNode = &pathNode;
    ret = ZipWriterEntryOpen(&param);
    if (ret == ARCHIVE_OK) {
        ret = ZipWriterCloseStream(&param, 0);
    }
    ZipWriterDeleteStream(&param);
    if (ZipWriterUpdateProgress(writer, 0, 0) == OH_ARCHIVE_PROGRESS_CANCEL) {
        return ARCHIVE_CANCEL_ERROR;
    }
    return ret;
}

static int ZipWriterAddOnePath(HmArchiveWriteInfo *archive, struct ZipWriter *writer, const char *infile,
                               const char *inputDir)
{
    int ret = ARCHIVE_OK;
    DirItemQueue dirItemQueue;
    InitDirItemQueue(&dirItemQueue);
    PushDirItemQueueNode(&dirItemQueue, infile);
    while ((ret == ARCHIVE_OK) && !IsDirItemQueueEmpty(&dirItemQueue)) {
        char *path = PopDirItemQueueNode(&dirItemQueue);
        if (IsDir(path)) {
            if (IsValidFolderEnd(path)) {
                ret = ZipWriterAddEmptyDir(archive, writer, inputDir, path);
            }
            if (ret == ARCHIVE_OK) {
                ret = TraversalDir(&dirItemQueue, path);
            }
        } else {
            FilePathNode pathNode;
            pathNode.filePath = path;
            ret = GetSaveName(path, inputDir, (const char **)&pathNode.saveName);
            if (ret == ARCHIVE_OK) {
                ret = ZipWriterAddOneFile(archive, writer, &pathNode);
            }
        }
        free(path);
    }
    FreeDirItemQueue(&dirItemQueue);
    return ret;
}

static int32_t ZipWriterAddFiles(HmArchiveWriteInfo *archive, struct ZipWriter *writer, const char **infile,
                                 size_t fileNum)
{
    int ret = ARCHIVE_OK;
    for (size_t i = 0; i < fileNum; i++) {
        char inputCutDir[ZIP_FILE_NAME_LEN_MAX + 1];
        GetprefixOfInput(infile[i], inputCutDir);
        ret = ZipWriterAddOnePath(archive, writer, infile[i], inputCutDir);
        if (ret != ARCHIVE_OK) {
            break;
        }
    }
    return ret;
}

static void ZipWriterDeleteFiles(HmArchiveWriteInfo *archive, struct ZipWriter *writer, int ret)
{
    if (ret != ARCHIVE_OK && archive->append == ARCHIVE_APPEND_STATUS_CREATE) {
        if (writer->baseStream != NULL) {
            StreamClose(writer->baseStream);
            StreamDestroy(&writer->baseStream);
        }
        if (remove(writer->outfile) != 0) {
            ARCHIVE_DEBUG("remove file fail\n");
        }
    }
}

static int ZipWriterAddFuncCheckWriter(struct ZipWriter *writer)
{
    if (writer == NULL || writer->baseStream == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipWriterAddFunc(HmArchiveWriteInfo *archive, const char **infile, uint64_t fileNum)
{
    if (archive == NULL || infile == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    struct ZipWriter *writer = (struct ZipWriter *)archive->fmtInfo;
    if (ZipWriterAddFuncCheckWriter(writer) != ARCHIVE_OK) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret = ARCHIVE_OK;
    if (!writer->isOpened) {
        ret = ZipWriterOpenOutputFile(archive);
        if (ret != ARCHIVE_OK) {
            return ret;
        }
        writer->isOpened = 1;
    }
    for (size_t i = 0; i < fileNum; i++) {
        if (infile[i] == NULL) {
            return ARCHIVE_PARAM_ERROR;
        }
        struct stat statBuf;
        if (stat(infile[i], &statBuf) != 0) {
            return ARCHIVE_EXIST_ERROR;
        }
    }

    ret = InitWriterProgress(archive, writer, infile, fileNum);
    if (ret == ARCHIVE_OK) {
        ret = ZipWriterAddFiles(archive, writer, infile, fileNum);
    }
    if (ret == ARCHIVE_OK) {
        ret = ZipWriterUpdateProgress(writer, 0, 1);
        ret = (ret == OH_ARCHIVE_PROGRESS_CANCEL) ? ARCHIVE_CANCEL_ERROR : ARCHIVE_OK;
    }
    ZipWriterDeleteFiles(archive, writer, ret);
    return ret;
}

static int ZipWriterZip64EndOfCD(struct ZipWriter *writer, struct Stream *stream)
{
    struct EndOfCentralDir64 endOfCentralDir64;
    endOfCentralDir64.signature = ZIP_MAGIC_ENDHEADER64;
    endOfCentralDir64.size = SETV((uint64_t)ZIP_END_OF_CD64_SIZE);
    endOfCentralDir64.versionMadeBy = SETV(writer->versionMadeBy);
    endOfCentralDir64.versionNeeded = SETV(ZIP_VERSION_45);
    endOfCentralDir64.numberOfDisk = SETV(writer->numberOfDiskWithCD);
    endOfCentralDir64.numberOfDiskWithCD = SETV(writer->numberOfDiskWithCD);
    endOfCentralDir64.totalNumberOfEntriesOnThisDisk = SETV(writer->totalNumberOfEntries);
    endOfCentralDir64.totalNumberOfEntries = SETV(writer->totalNumberOfEntries);
    endOfCentralDir64.sizeOfCentralDir = SETV(writer->sizeOfCentralDir);
    endOfCentralDir64.offsetOfCentralDir = SETV(writer->offsetOfCentralDir);
    int64_t writeRet = StreamWrite(stream, &endOfCentralDir64, sizeof(struct EndOfCentralDir64));
    if (writeRet != (int64_t)sizeof(struct EndOfCentralDir64)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipWriterZip64EndOfCDLocator(struct ZipWriter *writer, struct Stream *stream)
{
    struct EndOfCentralDirLocator64 endOfCentralDirLocator64;
    endOfCentralDirLocator64.signature = SETV(ZIP_MAGIC_ENDLOCHEADER64);
    endOfCentralDirLocator64.numberOfDiskWithEOCD64 = SETV(writer->numberOfDiskWithCD);
    endOfCentralDirLocator64.offsetOfEOCD64 = SETV(writer->offsetOfEOCD);
    endOfCentralDirLocator64.diskTotalNum = SETV((uint16_t)1);
    int64_t writeRet = StreamWrite(stream, &endOfCentralDirLocator64, sizeof(struct EndOfCentralDirLocator64));
    if (writeRet != (int64_t)sizeof(struct EndOfCentralDirLocator64)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipWriteEndOfCD(struct ZipWriter *writer, struct Stream *stream)
{
    uint16_t totalNumberOfEntries;
    uint32_t offsetOfCentralDir;
    uint32_t sizeOfCentralDir;
    sizeOfCentralDir = writer->sizeOfCentralDir > UINT32_MAX ? UINT32_MAX : writer->sizeOfCentralDir;
    offsetOfCentralDir = writer->offsetOfCentralDir > UINT32_MAX ? UINT32_MAX : writer->offsetOfCentralDir;
    totalNumberOfEntries = writer->totalNumberOfEntries > UINT16_MAX ? UINT16_MAX : writer->totalNumberOfEntries;

    struct EndOfCentralDir endOfCentralDir;
    endOfCentralDir.signature = SETV(ZIP_MAGIC_ENDHEADER);
    endOfCentralDir.numberOfDisk = SETV(writer->numberOfDiskWithCD);
    endOfCentralDir.numberOfDiskWithCD = SETV(writer->numberOfDiskWithCD);
    endOfCentralDir.totalNumberOfEntries = SETV(totalNumberOfEntries);
    endOfCentralDir.totalNumberOfEntriesOnThisDisk = SETV(totalNumberOfEntries);
    endOfCentralDir.sizeOfCentralDir = SETV(sizeOfCentralDir);
    endOfCentralDir.offsetOfCentralDir = SETV(offsetOfCentralDir);
    endOfCentralDir.commentLength = SETV(writer->globalCommentLength);
    int64_t writeRet = StreamWrite(stream, &endOfCentralDir, sizeof(struct EndOfCentralDir));
    if (writeRet != sizeof(struct EndOfCentralDir)) {
        return writeRet < 0 ? writeRet : ARCHIVE_WRITE_ERROR;
    }
    return ARCHIVE_OK;
}

static int ZipWriterSaveCD(struct ZipWriter *writer)
{
    int ret;
    if (writer->baseStream == NULL || writer->cdMemStream == NULL) {
        return ARCHIVE_OK;
    }
    struct Stream *baseStream = writer->baseStream;
    struct Stream *cdMemStream = writer->cdMemStream;
    writer->offsetOfCentralDir = (uint64_t)StreamTell(baseStream);
    ret = ZipWriteMemStream(baseStream, cdMemStream);
    RETURN_IF_FAIL(ret);
    writer->numberOfDiskWithCD = 0;
    writer->sizeOfCentralDir = (uint64_t)StreamTell(baseStream) - writer->offsetOfCentralDir;
    if (writer->sizeOfCentralDir > UINT32_MAX || writer->offsetOfCentralDir > UINT32_MAX ||
        writer->totalNumberOfEntries > UINT16_MAX) {
        writer->offsetOfEOCD = (uint64_t)StreamTell(baseStream);
        ret = ZipWriterZip64EndOfCD(writer, baseStream);
        RETURN_IF_FAIL(ret);

        ret = ZipWriterZip64EndOfCDLocator(writer, baseStream);
        RETURN_IF_FAIL(ret);
    }
    ret = ZipWriteEndOfCD(writer, baseStream);
    RETURN_IF_FAIL(ret);
    return ARCHIVE_OK;
}

static int ZipWriterClose(HmArchiveWriteInfo *archive)
{
    if (archive == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    struct ZipWriter *writer = (struct ZipWriter *)archive->fmtInfo;
    if (writer == NULL) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret = ARCHIVE_OK;
    if (writer->isOpened) {
        ret = ZipWriterSaveCD(writer);
    }
    if (writer->cdMemStream) {
        StreamClose(writer->cdMemStream);
        StreamDestroy(&writer->cdMemStream);
        writer->cdMemStream = NULL;
    }
    if (writer->baseStream) {
        StreamClose(writer->baseStream);
        StreamDestroy(&writer->baseStream);
        writer->baseStream = NULL;
    }
    if (writer->globalComment) {
        free(writer->globalComment);
        writer->globalComment = NULL;
    }
    free(archive->fmtInfo);
    archive->fmtInfo = NULL;
    return ret;
}

const FmtWriterOps g_ZipWriterFmtOps = {
    .open = ZipWriterOpenFile,
    .add = ZipWriterAddFunc,
    .close = ZipWriterClose,
};
