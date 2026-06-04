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

#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <gtest/gtest.h>
#include <clocale>
#include "oh_archive.h"
#include "zlib.h"
#include "oh_archive_plugin.h"

static OH_Archive_ProgressType ProgressHandler(int progress, void *userData)
{
    if (progress > 0) {
        printf("\rProgress: %d%%\n", progress);
    }
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressEmptyDir)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/zip-empty-dir.zip";
    const char *outDir = "/data/test/test_archive_reader_empty_dir";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressEmptyFile)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/zip-empty.zip";
    const char *outDir = "/data/test/test_archive_reader_empty_file";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressNormal)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/default.zip";
    const char *outDir = "/data/test/test_archive_reader_default_file";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressNormalCase2)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/other.zip";
    const char *outDir = "/data/test/test_archive_reader_files";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressFileNotExist)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/file_not_exist.zip";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_EQ(nullptr, arc);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressChineseName)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/中文名测试.zip";
    const char *outDir = "/data/test/test_archive_reader_chinese_name";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc, progressHandlerFunc,
        static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

static OH_Archive_ProgressType ProgressHandlerWithCancel(int progress, void *userData)
{
    printf("\rProgress: %d%%\n", progress);
    if (progress > 50) {    // 进度大于50
        return OH_ARCHIVE_PROGRESS_CANCEL;
    }
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressProgressCancel)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/default.zip";
    const char *outDir = "/data/test/test_archive_reader_zip_decompress_cancel";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandlerWithCancel;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_CANCEL_ERROR, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressProgressCancelAndDelete)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();

    const char *inFile = "/data/test/big_zip64.zip";
    const char *outDir = "/data/test/test_archive_reader_zip_decompress_cancel_and_delete";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandlerWithCancel;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_CANCEL_ERROR, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    int ret2 = access("/data/test/test_archive_reader_zip_decompress_cancel_and_delete/test_big_zip64_file",
        F_OK);
    EXPECT_NE(0, ret2);
}

TEST(ArchiveReadOpenSourceTest, ZipDecompressZip64File)
{
    GetHispeedArchivePluginHandle();
    ReleaseHispeedArchivePluginHandle();
    
    const char *inFile = "/data/test/big_zip64.zip";
    const char *outDir = "/data/test/test_archive_reader_zip_decompress_zip64File";

    OH_Archive_Reader_Ctx arc = OH_Archive_Reader_OpenFile(inFile);
    ASSERT_NE(nullptr, arc);

    OH_Archive_ProgressHandlerWithData progressHandlerFunc = ProgressHandler;
    int userData[2] = {0}; // 解压进程回调函数传递的用户自定义数据
    OH_Archive_ErrCode ret  = OH_Archive_Reader_SetProgressHandlerWithData(arc,
        progressHandlerFunc, static_cast<void*>(userData));
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_ExtractAllFile(arc, outDir);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);

    ret = OH_Archive_Reader_Close(arc);
    EXPECT_EQ(OH_ARCHIVE_OK, ret);
}