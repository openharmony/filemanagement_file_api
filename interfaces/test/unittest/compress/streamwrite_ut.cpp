/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <cstdio>
#include <climits>
#include <cstdint>
#include "oh_archive.h"
#include "oh_archive_errcode.h"
#include "zlib.h"
#include "oh_archive_plugin.h"
#include "oh_archive.h"

#define ULL unsigned long long
static ULL g_seed = 123456789LL;

static inline ULL IRand(void)
{
    g_seed = (g_seed * 0x41C64E6D + 0x3039) & 0x7FFFFFFF;
    return g_seed;
}

static int TestRandomInt(int min, int max)
{
    if (max <= min) {
        return min;
    }
    return IRand() % (max - min) + min;
}

static void FillBufferWithRandomData(void *buff, ULL size)
{
    if (!buff) {
        return;
    }
    char *p = static_cast<char *>(buff);
    for (ULL i = 0; i < size; i++) {
        p[i] = static_cast<char>(TestRandomInt(33, 126)); // 33-126 字符范围
    }
}

static void CreateRandomFile(const char *fileName, size_t fileSize)
{
    if (fileSize == 0) {
        return;
    }
    void *fileData = static_cast<void *>(malloc(fileSize * sizeof(char)));
    if (fileData == nullptr) {
        return;
 	}
    FillBufferWithRandomData(fileData, fileSize);

    FILE *file = fopen(fileName, "wb");
    (void)fwrite(fileData, 1, fileSize, file);
    (void)fclose(file);
    free(fileData);
}

static uint64_t PrintDataSizeReturnWrong(const void *data, uint64_t size, void *userData)
{
    printf("Data ptr: %p, data size: %llu\n", data, size);
    return static_cast<uint64_t>(-1);
}

static uint64_t WriteCallBack(const void *data, uint64_t size, void *userData)
{
    FILE *file = static_cast<FILE *>(userData);
    return fwrite(data, 1, size, file);
}

static void ErrorExit(const char *err, int code)
{
    (void)fprintf(stderr, "%s, code: %d\n", err, code);
    exit(code);
}

static int ReadFileData(const char *fileName, char **data, uint64_t *dataLength)
{
    FILE *file = fopen(fileName, "rb");
    if (file == nullptr) {
        return -1;
    }
    fseeko(file, 0, SEEK_END);
    int64_t fileSize = ftello(file);
    char *buffer = static_cast<char *>(malloc(fileSize * sizeof(char)));
    if (buffer == nullptr) {
        return -1;
    }
    fseeko(file, 0, SEEK_SET);
    (void)fread(buffer, 1, fileSize, file);
    (void)fclose(file);
    *data = buffer;
    *dataLength = fileSize;
    return 0;
}

static int WriteDataToFile(const char *fileName, const char *data, uint64_t dataLength)
{
    FILE *file = fopen(fileName, "wb");
    if (file == nullptr) {
        return -1;
    }
    int64_t writeSize = fwrite(data, 1, dataLength, file);
    if (writeSize != dataLength) {
        return -1;
    }
    (void)fclose(file);
    return 0;
}

#define CHUNK_SIZE 1048576

static int ZlibDecompressCore(const unsigned char *input, uint64_t inputLength, FILE *outFile, int windowBits)
{
    int ret;
    z_stream stream = {0};
    unsigned char buffer[CHUNK_SIZE];
    uint64_t inputOffset = 0;

    ret = inflateInit2(&stream, windowBits);
    if (ret != Z_OK) {
        return ret;
    }

    do {
        if (stream.avail_in == 0 && inputOffset < inputLength) {
            uint64_t remaining = inputLength - inputOffset;
            unsigned int chunkSize = remaining > UINT_MAX ? UINT_MAX : remaining;

            stream.avail_in = chunkSize;
            stream.next_in = (Bytef *)(input + inputOffset);
            inputOffset += chunkSize;
        }

        do {
            stream.avail_out = CHUNK_SIZE;
            stream.next_out = buffer;

            ret = inflate(&stream, Z_NO_FLUSH);

            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                    [[fallthrough]];
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_STREAM_ERROR:
                    inflateEnd(&stream);
                    [[fallthrough]];
                default:
                    return ret;
            }
            unsigned int have = CHUNK_SIZE - stream.avail_out;
            if (have > 0) {
                if (fwrite(buffer, 1, have, outFile) != have || ferror(outFile)) {
                    inflateEnd(&stream);
                    return Z_ERRNO;
                }
            }
        } while (stream.avail_out == 0);
    } while (ret != Z_STREAM_END && inputOffset < inputLength);

    inflateEnd(&stream);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

static int DecompressBufferToFile(const unsigned char *inputBuffer, uint64_t bufferLen, const char *outputFileName,
                                  int windowBits)
{
    if (!inputBuffer || !outputFileName) {
        return -1;
    }
    FILE *outFile = fopen(outputFileName, "wb");
    if (!outFile) {
        return -1;
    }

    int result = ZlibDecompressCore(inputBuffer, bufferLen, outFile, windowBits);
    (void)fclose(outFile);
    if (result != Z_OK) {
        return -1;
    }
    return 0;
}

static int DecompressFileToFile(const char *inputFileName, const char *outputFileName, int windowBits)
{
    char *inputData = nullptr;
    uint64_t inputDataLen = 0;

    if (ReadFileData(inputFileName, &inputData, &inputDataLen) != 0) {
        return -1;
    }

    int result = DecompressBufferToFile(reinterpret_cast<const unsigned char *>(inputData), inputDataLen,
                                        outputFileName, windowBits);
    free(inputData);
    if (result != 0) {
        return -1;
    }
    return result;
}

#define BUFFER_SIZE (64 * 1024)

static bool IsSameFile(const char *path1, const char *path2)
{
    FILE *f1 = fopen(path1, "rb");
    FILE *f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        return false;
    }
    (void)fseek(f1, 0, SEEK_END);
    (void)fseek(f2, 0, SEEK_END);
    if (ftell(f1) != ftell(f2)) {
        (void)fclose(f1);
        (void)fclose(f2);
        return false;
    }

    rewind(f1);
    rewind(f2);

    char buf1[BUFFER_SIZE];
    char buf2[BUFFER_SIZE];
    uint64_t byte1;
    uint64_t byte2;

    while ((byte1 = fread(buf1, 1, BUFFER_SIZE, f1)) > 0) {
        byte2 = fread(buf2, 1, BUFFER_SIZE, f2);
        if (byte2 != byte1 || memcmp(buf1, buf2, byte1) != 0) {
            (void)fclose(f1);
            (void)fclose(f2);
            return false;
        }
    }
    (void)fclose(f1);
    (void)fclose(f2);
    return true;
}

int DecompressFileBase(const char *inFile, const char *outFile)
{
    return DecompressFileToFile(inFile, outFile, -15);  // 窗口大小为-15
}

class OHCompressTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        config.blockSize = 32768; // 32768 bytes 为默认block大小
        config.threadNum = 4; // 默认 4 线程
        config.method = OH_ARCHIVE_COMPRESS_DEFLATE;
        config.checksum = OH_ARCHIVE_NO_CHECKSUM;

        GetHispeedArchivePluginHandle();
        ReleaseHispeedArchivePluginHandle();
    }

    void TearDown() override
    {
        if (ctx) {
            OH_Archive_StreamWrite_Destroy(ctx);
            ctx = nullptr;
        }
    }

    OH_Archive_ErrCode CompressFileCommon(const char *inFile, const char *outFile, int level, uint32_t quitThreshold,
                                          OH_Archive_StreamInfo *info)
    {
        FILE *fout = fopen(outFile, "wb");
        if (!fout) {
            return OH_ARCHIVE_OPEN_ERROR;
        }
        OH_Archive_StreamWrite_SetCompressLevel(ctx, level);
        OH_Archive_StreamWrite_Start(ctx, WriteCallBack, fout);
        const int bufferSize = 32 * 1024 * 4 + 2;
        unsigned char buffer[bufferSize];
        uint64_t totalSize = 0;
        FILE *fi = fopen(inFile, "rb");
        if (!fi) {
            (void)fclose(fout);
            return OH_ARCHIVE_OPEN_ERROR;
        }
        (void)fseek(fi, 0, SEEK_END);
        (void)fseek(fi, 0, SEEK_SET);

        OH_Archive_ErrCode result = OH_ARCHIVE_OK;
        uint64_t read = 0;
        while ((read = fread(buffer, 1, bufferSize, fi)) > 0) {
            totalSize += read;
            if (quitThreshold != 0 && totalSize >= quitThreshold) {
                result = OH_Archive_StreamWrite_Cancel(ctx);
                break;
            } else {
                result = OH_Archive_StreamWrite_Update(ctx, buffer, read);
                if (result != OH_ARCHIVE_OK) {
                    break;
                }
            }
        }
        (void)fclose(fi);
        if (result != OH_ARCHIVE_OK) {
            printf("CompressFileCommon failed, result: %d\n", result);
        }

        result = OH_Archive_StreamWrite_End(ctx, info);
        (void)fclose(fout);
        return result;
    }

    OH_Archive_ErrCode CompressFileAndGetInfo(const char *inFile, const char *outFile, int level,
                                              OH_Archive_StreamInfo *info)
    {
        return CompressFileCommon(inFile, outFile, level, 0, info);
    }

    OH_Archive_ErrCode CompressFile(const char *inFile, const char *outFile, int level)
    {
        return CompressFileCommon(inFile, outFile, level, 0, nullptr);
    }

    OH_Archive_ErrCode CompressFileQuit(const char *inFile, const char *outFile, int level, uint32_t quitThreshold)
    {
        return CompressFileCommon(inFile, outFile, level, quitThreshold, nullptr);
    }

    int CompressFileBase(const char *sourcePath, const char *targetPath, int level)
    {
        char *input = nullptr;
        uint64_t inputLen;
        int ret = ReadFileData(sourcePath, &input, &inputLen);
        if (ret != 0) {
            return ret;
        }

        z_stream stream = {0};

        char *output = static_cast<char *>(malloc(2 * inputLen * sizeof(char)));  // 2 倍输入大小
        if (output == nullptr) {
            return Z_MEM_ERROR;
        }
        stream.next_out = (Bytef *)output;
        stream.avail_out = 2 * static_cast<uInt>(inputLen);  // 2 倍输入大小
        stream.next_in = (z_const Bytef *)input;
        stream.avail_in = static_cast<uInt>(inputLen);

        int result = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
        if (result != Z_OK) {
            ErrorExit("deflateInit2 failed, result: %d\n", result);
        }

        result = deflate(&stream, Z_FINISH);
        if (result != Z_STREAM_END) {
            return result;
        }
        if (stream.avail_in > 0) {
            ErrorExit("compress: input was not consumed\n", Z_DATA_ERROR);
        }
        unsigned int outputLen = stream.total_out;

        result |= deflateEnd(&stream);
        if (result != Z_STREAM_END) {
            ErrorExit("deflateEnd failed, result: %d\n", result);
        }
        WriteDataToFile(targetPath, output, outputLen);
        free(input);
        free(output);
        return 0;
    }

    OH_Archive_Stream_Config config;
    OH_Archive_StreamWrite_Ctx ctx = nullptr;
};

TEST_F(OHCompressTest, CreateCompressCtxWithInvalidBlockSize)
{
    config.blockSize = 0;
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_EQ(ctx, nullptr);
}

TEST_F(OHCompressTest, CreateCompressCtxWithInvalidMethod)
{
    config.method = (OH_Archive_CompressMethod)(-1);
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_EQ(ctx, nullptr);
}

TEST_F(OHCompressTest, CreateCompressCtxWithInvalidCheckSum)
{
    config.checksum = (OH_Archive_StreamChecksum)(-1);
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_EQ(ctx, nullptr);
}

TEST_F(OHCompressTest, CompressCtxWithInvalidCallBack)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_NE(ctx, nullptr);

    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_Start(ctx, PrintDataSizeReturnWrong, nullptr);
    ASSERT_EQ(ret, OH_ARCHIVE_OK);

    const uint8_t data[4] = {0};    // 4 bytes 数据
    ret = OH_Archive_StreamWrite_Update(ctx, data, 4);
    ret = OH_Archive_StreamWrite_End(ctx, nullptr);
    ASSERT_EQ(ret, OH_ARCHIVE_STREAM_OUTPUT_ERROR);
}

TEST_F(OHCompressTest, CompressNormal)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    ASSERT_NE(ctx, nullptr);

    const char *compressedFile = "compressed_file_normal";
    const char *decompressedFile = "decompressed_file_normal";

    const char *inFile = "file_normal";
    CreateRandomFile(inFile, 1024 * 1024);  // 1024K大小

    int level = 5;
    OH_Archive_ErrCode compRet = CompressFile(inFile, compressedFile, level);
    ASSERT_EQ(compRet, OH_ARCHIVE_OK);
    int ret = DecompressFileBase(compressedFile, decompressedFile);
    EXPECT_EQ(ret, 0);
    bool isSame = IsSameFile(inFile, decompressedFile);
    EXPECT_TRUE(isSame);
}

TEST_F(OHCompressTest, CompressCheckSum)
{
    config.checksum = OH_ARCHIVE_CRC32;
    ctx = OH_Archive_StreamWrite_Create(config);
    ASSERT_NE(ctx, nullptr);

    const char *compressedFile = "compressed_file_normal";
    const char *decompressedFile = "decompressed_file_normal";

    const char *inFile = "file_normal";
    CreateRandomFile(inFile, 1024 * 1024);  // 1024K大小文件

    int level = 6;
    OH_Archive_ErrCode compRet = CompressFile(inFile, compressedFile, level);
    ASSERT_EQ(compRet, OH_ARCHIVE_OK);
    int ret = DecompressFileBase(compressedFile, decompressedFile);
    EXPECT_EQ(ret, 0);
    bool isSame = IsSameFile(inFile, decompressedFile);
    EXPECT_TRUE(isSame);
}

TEST_F(OHCompressTest, CompressCancel)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    ASSERT_NE(ctx, nullptr);

    const char *compressedFile = "compressed_file_abort";
    const char *decompressedFile = "decompressed_file_abort";

    const char *inFile = "file_abort";
    CreateRandomFile(inFile, 1024 * 1024); // 1024K大小

    int level = 6;
    CompressFileQuit(inFile, compressedFile, level, 2000); // 2000 bytes时退出
    int ret = DecompressFileBase(compressedFile, decompressedFile);
    EXPECT_NE(ret, 0);
    bool isSame = IsSameFile(inFile, decompressedFile);
    EXPECT_FALSE(isSame);
}

TEST_F(OHCompressTest, BufferCompress)
{
    const char *inFile = "file_buffer";
    CreateRandomFile(inFile, 1024 * 1024);  // 1024K大小文件
    char *source = nullptr;
    uint64_t sourceLen;
    ReadFileData(inFile, &source, &sourceLen);

    uint64_t destLen = 0;
    uint8_t *dest = nullptr;

    uint64_t bound = OH_Archive_BufferWriteCompressBound(OH_ARCHIVE_COMPRESS_DEFLATE, sourceLen);
    EXPECT_NE(bound, 0);

    dest = new uint8_t[bound];
    destLen = bound;

    int ret = OH_Archive_BufferWrite(dest, &destLen, reinterpret_cast<uint8_t *>(source), sourceLen,
                                     OH_ARCHIVE_COMPRESS_DEFLATE, 0);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    EXPECT_GT(destLen, 0u);

    free(source);
    delete[] dest;
}

TEST_F(OHCompressTest, CompressCancelNull)
{
    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_Cancel(nullptr);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, CompressCancelBeforeStart)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_NE(ctx, nullptr);

    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_Cancel(nullptr);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, SetLevelCtxIsNull)
{
    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_SetCompressLevel(nullptr, 5);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, SetInvalidLevel)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_NE(ctx, nullptr);

    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_SetCompressLevel(nullptr, 100);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, SetLevelAfterStart)
{
    ctx = OH_Archive_StreamWrite_Create(config);
    EXPECT_NE(ctx, nullptr);

    OH_Archive_ErrCode ret = OH_Archive_StreamWrite_Start(ctx, WriteCallBack, nullptr);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    ret = OH_Archive_StreamWrite_SetCompressLevel(ctx, -1);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, BufferCompressInvalidParam)
{
    OH_Archive_ErrCode ret = OH_Archive_BufferWrite(nullptr, nullptr, nullptr, 0, OH_ARCHIVE_COMPRESS_DEFLATE, 0);
    EXPECT_EQ(ret, OH_ARCHIVE_PARAM_ERROR);
}

TEST_F(OHCompressTest, BufferCompressBoundInvalidParam)
{
    uint64_t bound = OH_Archive_BufferWriteCompressBound((OH_Archive_CompressMethod) - 1, 10);
    EXPECT_EQ(bound, 0);
}