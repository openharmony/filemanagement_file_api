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
#include "oh_archive.h"
#include "oh_archive_errcode.h"
#include "unistd.h"
#include "oh_archive_plugin.h"
OH_Archive_ProgressType progressHandler(int progress, void *userData)
{
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ARCHIVE_ZIP_WRITER_TEST_OPEN, test_archive_writer_zip_compress_progress)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();
    const char *file = "./4gfile";
    const char* infile[] = {file};
    const char* outfile = "4gfile.zip";
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    FILE *fileHandle = fopen("./4gfile", "w");
    ASSERT_NE(fileHandle, NULL);

    uint64_t size = 4.1 * 1024 * 1024;
    char buf[1024] = {0};
    uint64_t writeSize = 0;
    while (writeSize < size) {
        fwrite(buf, sizeof(char), 1024, fileHandle);
        writeSize += 1;
    }
    fclose(fileHandle);

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(outfile, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, NULL);
    ret = OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, NULL);
    // 设置压缩算法
    ret = OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    ret = OH_Archive_Writer_Add(ctx, infile, 1);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    ret = OH_Archive_Writer_Close(ctx);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
}

TEST(ARCHIVE_ZIP_WRITER_TEST_OPEN, test_archive_writer_zip_compress_kongdir)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();
    const char *file = "./kong";
    const char* infile[] = {file};
    const char* outfile = "kong.zip";
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    int result = mkdir(file, 0777);
    if (result == 0) {
        OH_Archive_Writer_Ctx ctx =
            OH_Archive_Writer_OpenFile(outfile, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
        ASSERT_NE(ctx, NULL);
        ret = OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, NULL);
        // 设置压缩算法
        ret = OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
        ret = OH_Archive_Writer_Add(ctx, infile, 1);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
        ret = OH_Archive_Writer_Close(ctx);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
    }
}