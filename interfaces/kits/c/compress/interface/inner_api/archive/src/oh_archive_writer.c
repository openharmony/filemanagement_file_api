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
#include "errorcode.h"
#include "oh_archive_writer.h"
#include "oh_archive_plugin.h"
#include "archive_macros.h"
#include "archive_inner.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#define ARCHIVE_DEFAULT_COMPRESS_LEVEL (6)
#define ARCHIVE_MAX_COMPRESS_LEVEL (9)
const FmtWriterOps *g_AllFmtWriterOps[] = {
    NULL,
    NULL,
    NULL
};

static ErrCodeMap WriterErrCodeMap[] = {
    {ARCHIVE_OK, OH_ARCHIVE_OK},
    {ARCHIVE_PARAM_ERROR, OH_ARCHIVE_PARAM_ERROR},
    {ARCHIVE_SUPPORT_ERROR, OH_ARCHIVE_UNSUPPORTED_ERROR},
    {ARCHIVE_FORMAT_ERROR, OH_ARCHIVE_UNSUPPORTED_ERROR},
    {ARCHIVE_MEM_ERROR, OH_ARCHIVE_MEM_ERROR},
    {ARCHIVE_OPEN_ERROR, OH_ARCHIVE_OPEN_ERROR},
    {ARCHIVE_WRITE_ERROR, OH_ARCHIVE_WRITE_ERROR},
    {ARCHIVE_READ_ERROR, OH_ARCHIVE_READ_ERROR},
    {ARCHIVE_NO_SPACE_ERROR, OH_ARCHIVE_NO_SPACE_ERROR},
    {ARCHIVE_EXIST_ERROR, OH_ARCHIVE_PATH_NOT_EXIST_ERROR},
    {ARCHIVE_NAME_TOO_LONG_ERROR, OH_ARCHIVE_NAME_TOO_LONG_ERROR},
    {ARCHIVE_DATA_ERROR, OH_ARCHIVE_DATA_ERROR},
    {ARCHIVE_CANCEL_ERROR, OH_ARCHIVE_CANCEL_ERROR},
    {ARCHIVE_CRC_ERROR, OH_ARCHIVE_CRC_ERROR},
    {ARCHIVE_INTERNAL_ERROR, OH_ARCHIVE_UNKNOWN_ERROR},
    {ARCHIVE_EPERM_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_EACCES_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_EIO_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR},
    {ARCHIVE_ENOENT_ERROR, OH_ARCHIVE_PATH_ACCESS_ERROR}
};

static OH_Archive_ErrCode WriteConvertErrCode(int ret)
{
    int count = sizeof(WriterErrCodeMap) / sizeof(WriterErrCodeMap[0]);

    for (int i = 0; i < count; i++) {
        if (WriterErrCodeMap[i].ret == ret) {
            return WriterErrCodeMap[i].errcode;
        }
    }
    return OH_ARCHIVE_UNKNOWN_ERROR;
}

static void LoadArchiveWriterFunc(OH_Archive_Writer_Ctx ctx)
{
    const HispeedArchivePlugin* plugin = GetHispeedArchivePluginHandle();
    if (plugin == NULL) {
        return;
    }
    const FmtWriterOps *zipWriterOps = &plugin->zipWriterOps;
    if (zipWriterOps->open == NULL || zipWriterOps->add == NULL || zipWriterOps->close == NULL) {
        return;
    }
    ctx->fmtOps = zipWriterOps;
}

ARCHIVE_API OH_Archive_Writer_Ctx OH_Archive_Writer_OpenFile(const char *outfile,
                                                             OH_Archive_OpenMode openMode,
                                                             OH_Archive_Format fmt)
{
    if (outfile == NULL) {
        return NULL;
    }
    if (openMode != OH_ARCHIVE_OPEN_MODE_CREATE) {
        return NULL;
    }
    
    if (fmt != OH_ARCHIVE_FMT_ZIP) {
        return NULL;
    }

    OH_Archive_Writer_Ctx archive = calloc(1, sizeof(HmArchiveWriteInfo));
    if (archive == NULL) {
        return NULL;
    }
    archive->append = openMode;
    LoadArchiveWriterFunc(archive);
    if (archive->fmtOps == NULL) {
        const FmtWriterOps *writerOps = g_AllFmtWriterOps[fmt];
        if (writerOps == NULL) {
            free(archive);
            return NULL;
        }
        archive->fmtOps = writerOps;
    }

    if (archive->fmtOps == NULL || archive->fmtOps->open == NULL) {
        free(archive);
        return NULL;
    }

    OH_Archive_ErrCode ret = WriteConvertErrCode(archive->fmtOps->open(archive, outfile));
    if (ret != OH_ARCHIVE_OK) {
        free(archive);
        return NULL;
    }
    archive->level = ARCHIVE_DEFAULT_COMPRESS_LEVEL;
    archive->method = OH_ARCHIVE_COMPRESS_DEFLATE;
    archive->progressHandler = NULL;
    return archive;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Writer_SetCompressMethod(OH_Archive_Writer_Ctx arc,
                                                                   OH_Archive_CompressMethod method,
                                                                   int32_t compressLevel)
{
    if (arc == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (method != OH_ARCHIVE_NO_COMPRESSION && method != OH_ARCHIVE_COMPRESS_DEFLATE) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    if (compressLevel == (-1)) {
        compressLevel = ARCHIVE_DEFAULT_COMPRESS_LEVEL;
    }
    if (method != OH_ARCHIVE_NO_COMPRESSION && (compressLevel < 0 || compressLevel > ARCHIVE_MAX_COMPRESS_LEVEL)) {
        return OH_ARCHIVE_PARAM_ERROR;
    }
    arc->method = method;
    return OH_ARCHIVE_OK;
}

ARCHIVE_API
OH_Archive_ErrCode OH_Archive_Writer_SetProgressHandlerWithData(OH_Archive_Writer_Ctx arc,
                                                                OH_Archive_ProgressHandlerWithData progressHandler,
                                                                void *userData)
{
    if (arc == NULL || progressHandler == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    arc->progressHandler = progressHandler;
    arc->progressHandlerUserData = userData;
    return OH_ARCHIVE_OK;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Writer_Add(OH_Archive_Writer_Ctx arc,
                                                     const char** infiles,
                                                     uint64_t fileNum)
{
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
     
    if (arc == NULL || infiles == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    if (arc->fmtOps != NULL) {
        ret = WriteConvertErrCode(arc->fmtOps->add(arc, infiles, fileNum));
    }
    return ret;
}

ARCHIVE_API OH_Archive_ErrCode OH_Archive_Writer_Close(OH_Archive_Writer_Ctx arc)
{
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
     
    if (arc == NULL) {
        return OH_ARCHIVE_PARAM_ERROR;
    }

    if (arc->fmtOps != NULL && arc->fmtOps->close != NULL) {
        ret = WriteConvertErrCode(arc->fmtOps->close(arc));
    }

    free(arc);
    return ret;
}