#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtest/gtest.h>
#include <locale.h>
#include "oh_archive.h"
#include "zlib.h"

static OH_Archive_ProgressType ProgressHandler(int progress, void *userData)
{
    if (progress > 0) {
        printf("\rProgress: %d%%\n", progress);
    }
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ArchiveReadTest, ZipDecompressEmptyDir)
{
    const char *inFile = "./data/zip-empty-dir.zip";
    const char *outDir = "./test_archive_reader_empty_dir";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, ZipDecompressEmptyFile)
{
    const char *inFile = "./data/zip-empty.zip";
    const char *outDir = "./test_archive_reader_empty_file";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, ZipDecompressNormal)
{
    const char *inFile = "./data/default.zip";
    const char *outDir = "./test_archive_reader_default_file";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, ZipDecompressFileNotExist)
{
    const char *inFile = "./data/file_not_exist.zip";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);
}

TEST(ArchiveReadTest, ZipDecompressChineseName)
{
    const char *inFile = "./data/中文名测试.zip";
    const char *outDir = "./test_archive_reader_chinese_name";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, ZipDecompressExtraField)
{
    const char *inFile = "./data/silesia_extra_field.zip";
    const char *outDir = "./test_archive_reader_zip_decompress_extra_field";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

static OH_Archive_ProgressType ProgressHandlerWithCancel(int progress, void *userData)
{
    printf("\rProgress: %d%%\n", progress);
    if (progress > 50) {
        return OH_ARCHIVE_PROGRESS_CANCEL;
    }
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ArchiveReadTest, ZipDecompressProgressCancel)
{
    const char *inFile = "./data/silesia.zip";
    const char *outDir = "./test_archive_reader_zip_decompress_cancel";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandlerWithCancel;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_CANCEL_ERROR, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, ZipDecompressProgressCancelAndDelete)
{
    const char *inFile = "./data/big_zip64.zip";
    const char *outDir = "./test_archive_reader_zip_decompress_cancel_and_delete";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandlerWithCancel;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_CANCEL_ERROR, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    int ret2 = access("./test_archive_reader_zip_decompress_cancel_and_delete/test_big_zip64_file",
        F_OK);
    EXPECT_NE(0, ret2);
}

TEST(ArchiveReadTest, ZipDecompressZip64File)
{
    const char *inFile = "./data/big_zip64.zip";
    const char *outDir = "./test_archive_reader_zip_decompress_zip64File";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(NULL, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, (void*)userData);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

#define ZLIB_OK 0;
#define ZLIB_ERROR 1;

static int TestCompressBase(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong left;

    left = *destLen;
    *destLen = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, -1, Z_DEFLATED, -15 , 6, 4);
    if (err != Z_OK) {
        return err;
    }

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }

        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    *destLen = stream.total_out;
    deflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}

static int TestCompress(const char *inFileName , const char *outFileName, uint32_t &crc)
{
    FILE *inFile, *outFile;
    inFile = fopen(inFileName, "rb");
    if (inFile == NULL) {
        return ZLIB_ERROR;
    }
    Byte *data = NULL;
    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    if (dataLen == 0) {
        return ZLIB_ERROR;
    }

    data = (Byte *)malloc(dataLen);
    if (data == NULL) {
        return ZLIB_ERROR;
    }

    rewind(inFile);
    fread(data, dataLen, 1, inFile);
    crc = crc32(0, data, dataLen);
    outFile = fopen(outFileName, "wb");
    if (outFile == NULL) {
        fclose(inFile);
        return ZLIB_ERROR;
    }

    Byte *compr = NULL;
    uLong comprLen = compressBound(dataLen);
    compr = (Bytef*)malloc(dataLen * sizeof(Bytef));
    if (compr == NULL) {
        free(data);
        fclose(inFile);
        fclose(outFile);
        return ZLIB_ERROR;
    }

    if (TestCompressBase(compr, &comprLen, (const Bytef*)data, dataLen) != Z_OK) {
        free(data);
        free(compr);
        fclose(inFile);
        fclose(outFile);
        return ZLIB_ERROR;
    }

    if (fwrite(compr, 1, comprLen, outFile) != comprLen || ferror(outFile)) {
        free(data);
        free(compr);
        fclose(inFile);
        fclose(outFile);
        return ZLIB_ERROR;
    }
    return ZLIB_OK;
}

OH_Archive_ErrCode TestBufferReadDecompress(const char *inFileName, const char *outFileName)
{
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    FILE *inFile, *outFile;
    inFile = fopen(inFileName, "rb");
    if (inFile == NULL) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }
    Byte *data = NULL;
    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    if (dataLen == 0) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    data = (Byte *)malloc(dataLen);
    if (data == NULL) {
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    rewind(inFile);
    fread(data, dataLen, 1, inFile);
    outFile = fopen(outFileName, "wb");
    if (outFile == NULL) {
        fclose(inFile);
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    Byte *uncompr = NULL;
    uLong uncomprLen = 1<<30;
    uncompr = (Bytef*)malloc(dataLen * sizeof(Bytef));
    if (uncompr == NULL) {
        free(data);
        fclose(inFile);
        fclose(outFile);
        return OH_ARCHIVE_DEFLATE_ERROR;
    }

    ret = OH_Archive_BufferRead(uncompr, &uncomprLen, (const Bytef*)data, dataLen,
        OH_ARCHIVE_COMPRESS_DEFLATE);
    free(data);
    free(uncompr);
    fclose(inFile);
    return ret;
}

TEST(ArchiveReadTest, BufferReadDecompressNormalCase1)
{
    uint32_t crc = 0;
    int ret = TestCompress("./data/gzip_txt", "./buffer_read_normal_case1", crc);
    EXPECT_EQ(Z_OK, ret);

    ret = TestBufferReadDecompress("./buffer_read_normal_case1", "./buffer_read_normal_case1_decompress");
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, BufferReadDecompressNormalCase2)
{
    uint32_t crc = 0;
    int ret = TestCompress("./data/utf32.txt", "./buffer_read_normal_case2", crc);
    EXPECT_EQ(Z_OK, ret);

    ret = TestBufferReadDecompress("./buffer_read_normal_case2", "./buffer_read_normal_case2_decompress");
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, BufferReadDecompressNormalCase3)
{
    uint32_t crc = 0;
    int ret = TestCompress("./data/dickens", "./buffer_read_normal_case3", crc);
    EXPECT_EQ(Z_OK, ret);

    ret = TestBufferReadDecompress("./buffer_read_normal_case3", "./buffer_read_normal_case3_decompress");
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadTest, BufferReadDecompressEmptyFile)
{
    OH_Archive_ErrCode ret = TestBufferReadDecompress("./data/aafgGWKC1E23.txt",
            "./buffer_read_normal_EmptyFile_decompress");
    EXPECT_EQ(OH_ARCHIVE_PARAM_ERROR, ret);
}

TEST(ArchiveReadTest, BufferReadDecompressOutBufInsuff)
{
    uint32_t crc = 0;
    int ret2 = TestCompress("./data/dickens", "./buffer_read_normal_case_outbuf_insuff", crc);
    EXPECT_EQ(Z_OK, ret2);

    const char *inFileName = "./buffer_read_normal_case_outbuf_insuff";
    const char *outFileName = "./buffer_read_normal_case_outbuf_insuff_decompress";

    FILE *inFile, *outFile;
    inFile = fopen(inFileName, "rb");
    ASSERT_NE(NULL, inFile);

    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    Byte *data = (Byte *)malloc(dataLen);
    ASSERT_NE(NULL, data);
    rewind(inFile);
    fread(data, dataLen, 1, inFile);

    crc = crc32(0, data, dataLen);
    outFile = fopen(outFileName, "wb");
    ASSERT_NE(NULL, outFile);

    Byte *uncompr = NULL;
    uLong uncomprLen = 10; // insufficient outbuf
    uncompr = (Bytef*)malloc(dataLen * sizeof(Bytef));
    ASSERT_NE(NULL, uncompr);

    OH_Archive_ErrCode ret = OH_Archive_BufferRead(uncompr, &uncomprLen, (const Bytef*)data, dataLen,
        OH_ARCHIVE_COMPRESS_DEFLATE);
    EXPECT_EQ(OH_ARCHIVE_INSUFFICIENT_OUTBUF_ERROR, ret);
    
    free(data);
    free(uncompr);
    fclose(inFile);
    fclose(outFile);
}

TEST(ArchiveReadTest, BufferReadDecompressNotSupportMethod)
{
    uLong dataLen = 10;
    Byte *data = (Bytef*)malloc(dataLen * sizeof(Bytef));
    ASSERT_NE(NULL, data);

    uLong uncomprLen = 10;
    Byte *uncompr = (Bytef*)malloc(uncomprLen * sizeof(Bytef));
    ASSERT_NE(NULL, uncompr);

    OH_Archive_ErrCode ret = OH_Archive_BufferRead(uncompr, &uncomprLen, (const Bytef*)data, dataLen,
        (OH_Archive_CompressMethod)7);
    EXPECT_EQ(OH_ARCHIVE_PARAM_ERROR, ret);
    
    free(data);
    free(uncompr);
}

uint64_t ReaderHandler(const void *data, uint64_t size, void* userData)
{
    const char *outFileName = "./data/stream_decompress";
    FILE *outFile = fopen(outFileName, "ab");
    if (outFile == NULL) {
        fclose(outFile);
        return 0;
    }

    if (fwrite(data, 1, size, outFile) != size) {
        return 0;
    }

    fclose(outFile);
    return size;
}

TEST(ArchiveReadTest, StreamReadDecompressNormalCase1)
{
    uint32_t crc = 0;
    int ret = TestCompress("./data/gzip_txt", "./stream_read_normal_case1", crc);
    EXPECT_EQ(Z_OK, ret);

    const char *inFileName = "./stream_read_normal_case1";
    FILE *inFile = fopen(inFileName, "rb");
    ASSERT_NE(NULL, inFile);

    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    uint8_t *data = (uint8_t *)malloc(dataLen * sizeof(uint8_t));
    ASSERT_NE(NULL, data);
    rewind(inFile);
    fread(data, dataLen, 1, inFile);

    OH_Archive_Stream_Config config = {0};
    config.blockSize = 32 * 1024;
    config.method = OH_ARCHIVE_COMPRESS_DEFLATE;
    config.checksum = OH_ARCHIVE_CRC32;
    OH_Archive_StreamRead_Ctx ctx = OH_Archive_StreamRead_Create(config);
    ASSERT_NE(NULL, ctx);

    OH_Archive_Stream_OutputHandler readerHandler = ReaderHandler;
    OH_Archive_ErrCode ret2 = OH_Archive_StreamRead_Start(ctx, readerHandler, NULL);
    EXPECT_EQ(OH_ARCHIVE_OK, ret2);

    ret2 = OH_Archive_StreamRead_Update(ctx, data, dataLen);
    EXPECT_EQ(OH_ARCHIVE_OK, ret2);

    OH_Archive_StreamInfo info = {0};
    ret2 = OH_Archive_StreamRead_End(ctx, &info);
    EXPECT_EQ(OH_ARCHIVE_OK, ret2);
    EXPECT_EQ(crc, info.checksum);

    OH_Archive_StreamRead_Destroy(ctx);

    fclose(inFile);
    free(data);
}

TEST(ArchiveReadTest, StreamReadDecompressCancel)
{
    uint32_t crc = 0;
    int ret2 = TestCompress("./data/gzip_txt", "./stream_read_cancel", crc);
    EXPECT_EQ(Z_OK, ret2);

    const char *inFileName = "./stream_read_cancel";
    FILE *inFile = fopen(inFileName, "rb");
    ASSERT_NE(NULL, inFile);

    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    uint8_t *data = (uint8_t *)malloc(dataLen * sizeof(uint8_t));
    ASSERT_NE(NULL, data);
    rewind(inFile);
    fread(data, dataLen, 1, inFile);

    OH_Archive_Stream_Config config = {0};
    config.blockSize = 32 * 1024;
    config.method = OH_ARCHIVE_COMPRESS_DEFLATE;
    config.checksum = OH_ARCHIVE_CRC32;
    OH_Archive_StreamRead_Ctx ctx = OH_Archive_StreamRead_Create(config);
    ASSERT_NE(NULL, ctx);

    OH_Archive_Stream_OutputHandler readerHandler = ReaderHandler;
    OH_Archive_ErrCode ret = OH_Archive_StreamRead_Start(ctx, readerHandler, NULL);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_StreamRead_Cancel(ctx);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_StreamRead_Update(ctx, data, dataLen);
    EXPECT_EQ(OH_ARCHIVE_CANCEL_ERROR, ret);

    ret = OH_Archive_StreamRead_End(ctx, NULL);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    OH_Archive_StreamRead_Destroy(ctx);

    fclose(inFile);
    free(data);
}

TEST(ArchiveReadTest, StreamReadDecompressNotSupportMethod)
{
    uint32_t crc = 0;
    int ret = TestCompress("./data/gzip_txt", "./stream_read_not_support_method", crc);
    EXPECT_EQ(Z_OK, ret);

    const char *inFileName = "./stream_read_not_support_method";
    FILE *inFile = fopen(inFileName, "rb");
    ASSERT_NE(NULL, inFile);

    fseek(inFile, 0, SEEK_END);
    uLong dataLen = ftell(inFile);
    uint8_t *data = (uint8_t *)malloc(dataLen * sizeof(uint8_t));
    ASSERT_NE(NULL, data);
    rewind(inFile);
    fread(data, dataLen, 1, inFile);

    OH_Archive_Stream_Config config = {0};
    config.blockSize = 32 * 1024;
    config.method = (OH_Archive_CompressMethod)7;
    config.checksum = OH_ARCHIVE_CRC32;
    OH_Archive_StreamRead_Ctx ctx = OH_Archive_StreamRead_Create(config);
    EXPECT_EQ(NULL, ctx);

    fclose(inFile);
    free(data);
}





