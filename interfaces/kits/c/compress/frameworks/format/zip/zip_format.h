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

#ifndef ZIP_FORMAT_H
#define ZIP_FORMAT_H

#include <stdint.h>

#define ZIP_FLAG_ENCRYPTED          (1 << 0)
#define ZIP_FLAG_LZMA_EOS_MARKER    (1 << 1)
#define ZIP_FLAG_DEFLATE_MAX        (1 << 1)
#define ZIP_FLAG_DEFLATE_NORMAL     (0)
#define ZIP_FLAG_DEFLATE_FAST       (1 << 2)
#define ZIP_FLAG_DEFLATE_SUPER_FAST (ZIP_FLAG_DEFLATE_FAST | ZIP_FLAG_DEFLATE_MAX)
#define ZIP_FLAG_DATA_DESCRIPTOR    (1 << 3)
#define ZIP_FLAG_STRONG_ENCRYPTED   (1 << 6)
#define ZIP_FLAG_UTF8               (1 << 11)
#define ZIP_FLAG_MASK_LOCAL_INFO    (1 << 13)

#define ZIP_HOST_SYSTEM(VERSION_MADEBY) ((uint8_t)((uint32_t)(VERSION_MADEBY) >> 8))
#define ZIP_HOST_SYSTEM_MSDOS           (0)
#define ZIP_HOST_SYSTEM_UNIX            (3)
#define ZIP_HOST_SYSTEM_WINDOWS_NTFS    (10)
#define ZIP_HOST_SYSTEM_RISCOS          (13)
#define ZIP_HOST_SYSTEM_OSX_DARWIN      (19)

#define ZIP_HOST_OS_FAT (0)
#define ZIP_HOST_OS_AMIGA (1)
#define ZIP_HOST_OS_VMS (2)
#define ZIP_HOST_OS_UNIX (3)
#define ZIP_HOST_OS_VM_CMS (4)
#define ZIP_HOST_OS_ATARI (5)
#define ZIP_HOST_OS_HPFS (6)
#define ZIP_HOST_OS_MAC (7)
#define ZIP_HOST_OS_Z_SYSTEM (8)
#define ZIP_HOST_OS_CPM (9)
#define ZIP_HOST_OS_TOPS20 (10)
#define ZIP_HOST_OS_NTFS (11)
#define ZIP_HOST_OS_QDOS (12)
#define ZIP_HOST_OS_ACORN (13)
#define ZIP_HOST_OS_VFAT (14)
#define ZIP_HOST_OS_MVS (15)
#define ZIP_HOST_OS_BEOS (16)
#define ZIP_HOST_OS_TANDEM (17)
#define ZIP_HOST_OS_OS400 (18)
#define ZIP_HOST_OS_OSX (19)

#if defined(__APPLE__)
#   define ZIP_VERSION_MADEBY_HOST_SYSTEM (ZIP_HOST_SYSTEM_OSX_DARWIN)
#elif defined(__riscos__)
#   define ZIP_VERSION_MADEBY_HOST_SYSTEM (ZIP_HOST_SYSTEM_RISCOS)
#elif defined(_WIN32)
#   define ZIP_VERSION_MADEBY_HOST_SYSTEM (ZIP_HOST_SYSTEM_WINDOWS_NTFS)
#else
#   define ZIP_VERSION_MADEBY_HOST_SYSTEM (ZIP_HOST_SYSTEM_UNIX)
#endif

#define ZIP_VERSION_MADEBY_HAVE_WZAES (51)

#if defined(HAVE_LZMA) || defined(HAVE_LIBCOMP)
#   define ZIP_VERSION_MADEBY_ZIP_VERSION (63)
#elif defined(HAVE_WZAES)
#   define ZIP_VERSION_MADEBY_ZIP_VERSION (51)
#elif defined(HAVE_BZIP2)
#   define ZIP_VERSION_MADEBY_ZIP_VERSION (46)
#else
#   define ZIP_VERSION_MADEBY_ZIP_VERSION (45)
#endif

#define ZIP_VERSION_MADEBY          ((ZIP_VERSION_MADEBY_HOST_SYSTEM << 8) |  \
                                    (ZIP_VERSION_MADEBY_ZIP_VERSION))

#define ZIP_MAGIC_LOCALHEADER           (0x04034b50)
#define ZIP_MAGIC_LOCALHEADERU8         { 0x50, 0x4b, 0x03, 0x04 }
#define ZIP_MAGIC_CENTRALHEADER         (0x02014b50)
#define ZIP_MAGIC_CENTRALHEADERU8       { 0x50, 0x4b, 0x01, 0x02 }
#define ZIP_MAGIC_ENDHEADER             (0x06054b50)
#define ZIP_MAGIC_ENDHEADERU8           { 0x50, 0x4b, 0x05, 0x06 }
#define ZIP_MAGIC_ENDHEADER64           (0x06064b50)
#define ZIP_MAGIC_ENDHEADER64U8         { 0x50, 0x4b, 0x06, 0x06 }
#define ZIP_MAGIC_ENDLOCHEADER64        (0x07064b50)
#define ZIP_MAGIC_ENDLOCHEADER64U8      { 0x50, 0x4b, 0x06, 0x07 }
#define ZIP_MAGIC_DATADESCRIPTOR        (0x08074b50)
#define ZIP_MAGIC_DATADESCRIPTORU8      { 0x50, 0x4b, 0x07, 0x08 }

#define ZIP_SIZE_LD_ITEM             (30)
#define ZIP_SIZE_CD_ITEM             (46)
#define ZIP_SIZE_CD_LOCATOR64        (20)
#define ZIP_SIZE_MAX_DATA_DESCRIPTOR (24)
#define ZIP_SIZE_EOCD_FIXED          (22)
#define ZIP_SIZE_SIGNATURE           (4)
#define ZIP_SIZE_MAX_GLOBAL_COMMENT  (0xFFFF)

#define ZIP_SIZE_MAX_EXTRA_FIELD_SIZE (0xFFFF)

#define ZIP_OFFSET_CRC_SIZES          (14)
#define ZIP_UNCOMPR_SIZE64_CUSHION    (2 * 1024 * 1024)
#define CRC_POS_IN_LOCAL_HEADER 14

#ifndef ZIP_EOCD_MAX_BACK
#define ZIP_EOCD_MAX_BACK             (1 << 20)
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 65535U
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 4294967295UL
#endif

#define ZIP64_AUTO          0
#define ZIP64_FORCE         1
#define ZIP64_DISABLE       2

#define ZIP_EXTRAFIELD_ZIP64_HEADER  0x0001
#define ZIP_LOCAL_EXTRAFIELD_ZIP64_LENGTH 16
#define ZIP_CD_EXTRAFIELD_ZIP64_LENGTH 24
#define ZIP_END_OF_CD64_SIZE 44

#pragma pack(1)

struct EndOfCentralDir {
    uint32_t signature;
    uint16_t numberOfDisk;
    uint16_t numberOfDiskWithCD;
    uint16_t totalNumberOfEntriesOnThisDisk;
    uint16_t totalNumberOfEntries;
    uint32_t sizeOfCentralDir;
    uint32_t offsetOfCentralDir;
    uint16_t commentLength;
};

struct EndOfCentralDirLocator64 {
    uint32_t signature;
    uint32_t numberOfDiskWithEOCD64;
    uint64_t offsetOfEOCD64;
    uint32_t diskTotalNum;
};

struct EndOfCentralDir64 {
    uint32_t signature;
    uint64_t size;
    uint16_t versionMadeBy;
    uint16_t versionNeeded;
    uint32_t numberOfDisk;
    uint32_t numberOfDiskWithCD;
    uint64_t totalNumberOfEntriesOnThisDisk;
    uint64_t totalNumberOfEntries;
    uint64_t sizeOfCentralDir;
    uint64_t offsetOfCentralDir;
};

struct DigitalSignature {
    uint32_t signature;
    uint16_t sizeOfData;
};

struct CentralDirHeader {
    uint32_t signature;
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

struct ArchiveExtraData {
    uint32_t signature;
    uint32_t extraFieldLength;
};

struct LocalExtraDataZIP64 {
    uint16_t signature;
    uint16_t extraFieldLength;
    uint64_t uncompressedSize;
    uint64_t compressedSize;
};

struct CdExtraDataZIP64 {
    uint16_t signature;
    uint16_t extraFieldLength;
};

struct DataSizeDescriptor {
    uint32_t compressedSize;
    uint32_t uncompressedSize;
};

struct DataDescriptor {
    uint32_t signature;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
};

struct DataSizeDescriptor64 {
    uint64_t compressedSize;
    uint64_t uncompressedSize;
};

struct LocalFileHeader {
    uint32_t signature;
    uint16_t versionNeeded;
    uint16_t flag;
    uint16_t compressionMethod;
    uint32_t dosModDateTime;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t fileNameLength;
    uint16_t extraFieldLength;
};

struct WinZipAesExtraFields {
    uint16_t version;
    uint8_t flag1;
    uint8_t flag2;
    uint8_t strength;
    uint16_t compressMethod;
};

struct WinZipAesHeader {
    uint16_t headerID;
    uint16_t dataSize;
    uint16_t version;
    uint8_t flag1;
    uint8_t flag2;
    uint8_t strength;
    uint16_t compressMethod;
};

struct StrongEncryptionExtraFields {
    uint16_t format;
    uint16_t algId;
    uint16_t keyLen;
    uint16_t flags;
};

struct StrongEncryptionHeader {
    uint16_t headerID;
    uint16_t size;
    uint16_t format;
    uint16_t algId;
    uint16_t keyLen;
    uint16_t flags;
};

#pragma pack()

#endif