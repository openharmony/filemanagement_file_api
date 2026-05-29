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
#include <unistd.h>
#include <securec.h>
#include "stream.h"
#include "stream_file.h"
#include "archive_macros.h"
#include "errorcode.h"
#define STREAM_MAGIC_FILE 0x4B7FA49F

typedef struct {
    struct Stream stream;
    char path[PATH_MAX + 1];
    FILE *handle;
} FileStream;

ARCHIVE_IMPL const char *FindLastSeparator(const char *filePath)
{
    const char *lastSeparator = NULL;
    const char *p;
    for (p = filePath; *p; p++) {
        if (*p == '/') {
            lastSeparator = p;
        }
    }
    return lastSeparator;
}

static int32_t ConvertWriteErrno(int errnoNum, int defaultErrNum)
{
    switch (errnoNum) {
        case ENOSPC:
            return ARCHIVE_NO_SPACE_ERROR;
        case EPERM:
            return ARCHIVE_EPERM_ERROR;
        case EIO:
            return ARCHIVE_EIO_ERROR;
        case EACCES:
            return ARCHIVE_EACCES_ERROR;
        case ENOENT:
            return ARCHIVE_ENOENT_ERROR;
        case ENAMETOOLONG:
            return ARCHIVE_NAME_TOO_LONG_ERROR;
        default:
            return defaultErrNum;
    }
}

ARCHIVE_IMPL int GetParentDirectory(const char *filePath, char *parentPath, size_t parentPathSize)
{
    if (!filePath || strlen(filePath) >= parentPathSize) {
        return ARCHIVE_PARAM_ERROR;
    }
    const char *lastSeparator = FindLastSeparator(filePath);
    if (!lastSeparator) {
        parentPath[0] = '.';
        parentPath[1] = '\0';
        return ARCHIVE_OK;
    }
    size_t parentLen = lastSeparator - filePath;
    if (parentLen >= ZIP_FILE_NAME_LEN_MAX) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret = strncpy_s(parentPath, ZIP_FILE_NAME_LEN_MAX, filePath, parentLen);
    if (ret != EOK) {
        return ARCHIVE_PARAM_ERROR;
    }
    parentPath[parentLen] = '\0';
    if (parentPath[0] == '\0') {
        parentPath[0] = '.';
        parentPath[1] = '\0';
        return ARCHIVE_OK;
    }
    return ARCHIVE_OK;
}

static const char *GetFileName(const char *filePath)
{
    const char *lastSeparator = FindLastSeparator(filePath);
    if (lastSeparator == NULL) {
        return filePath;
    }
    return lastSeparator + 1;
}

LOCAL int FileStreamOpen(struct Stream *stream, int mode)
{
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;
    const char *openMode = NULL;
    if (((unsigned)mode & OPEN_MODE_READ_WRITE) == OPEN_MODE_READ) {
        openMode = "rb";
    } else if ((unsigned)mode & OPEN_MODE_APPEND) {
        openMode = "r+b";
    } else if ((unsigned)mode & OPEN_MODE_CREATE) {
        openMode = "wb";
        if (access(file->path, F_OK) != -1) {
            remove(file->path);
        }
    } else {
        return ARCHIVE_OPEN_ERROR;
    }
    char resolvedPath[PATH_MAX + 1];
    if ((((unsigned)mode & OPEN_MODE_READ_WRITE) == OPEN_MODE_READ) ||
        ((unsigned)mode & OPEN_MODE_APPEND)) {
        if (realpath(file->path, resolvedPath) != NULL) {
            file->handle = fopen(resolvedPath, openMode);
        } else {
            return ConvertWriteErrno(errno, ARCHIVE_OPEN_ERROR);
        }
    } else {
        char parentPath[ZIP_FILE_NAME_LEN_MAX];
        char fullPath[ZIP_FILE_NAME_LEN_MAX];
        if (GetParentDirectory(file->path, parentPath, ZIP_FILE_NAME_LEN_MAX) != ARCHIVE_OK) {
            return ARCHIVE_OPEN_ERROR;
        }
        if (realpath(parentPath, resolvedPath) == NULL) {
            return ConvertWriteErrno(errno, ARCHIVE_OPEN_ERROR);
        }
        const char *fileName = GetFileName(file->path);
        if (snprintf_s(fullPath, ZIP_FILE_NAME_LEN_MAX, ZIP_FILE_NAME_LEN_MAX - 1,
                       "%s/%s", resolvedPath, fileName) < 0) {
            return ARCHIVE_OPEN_ERROR;
        }
        file->handle = fopen(fullPath, openMode);
    }
    if (file->handle == NULL) {
        return ConvertWriteErrno(errno, ARCHIVE_OPEN_ERROR);
    }
    return ARCHIVE_OK;
}

LOCAL int64_t FileStreamRead(struct Stream *stream, void *buf, int64_t len)
{
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;
    int64_t readSize = (int64_t)fread(buf, 1, len, file->handle);
    if (readSize != len && ferror(file->handle)) {
        return ConvertWriteErrno(errno, ARCHIVE_READ_ERROR);
    }
    return readSize;
}

LOCAL int64_t FileStreamWrite(struct Stream *stream, const void *buf, int64_t len)
{
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;
    ASSERT(file->handle);
    int64_t writeSize = (int64_t)fwrite(buf, 1, len, file->handle);
    if (writeSize != len && ferror(file->handle)) {
        return ConvertWriteErrno(errno, ARCHIVE_WRITE_ERROR);
    }
    return writeSize;
}

LOCAL int FileStreamClose(struct Stream *stream)
{
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;

    if (file->handle != NULL) {
        fclose(file->handle);
        file->handle = NULL;
    }

    return ARCHIVE_OK;
}

LOCAL int FileStreamSeek(struct Stream *stream, int64_t offset, int origin)
{
    RETURN_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;
    ASSERT(file->handle);
    int ret = fseek(file->handle, offset, origin);
    if (ret < 0) {
        return ConvertWriteErrno(errno, ARCHIVE_SEEK_ERROR);
    }
    return ret;
}

LOCAL int64_t FileStreamTell(struct Stream *stream)
{
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    FileStream *file = (FileStream *)(void *)stream;
    ASSERT(file->handle);
    int64_t tellPosition = ftell(file->handle);
    if (tellPosition == -1) {
        return ConvertWriteErrno(errno, ARCHIVE_TELL_ERROR);
    }
    return tellPosition;
}

LOCAL int FileStreamOpenNull(struct Stream *stream, int mode)
{
    ARCHIVE_UNUSED(stream);
    ARCHIVE_UNUSED(mode);
    return ARCHIVE_OK;
}

LOCAL int FileStreamCloseNull(struct Stream *stream)
{
    ARCHIVE_UNUSED(stream);
    return ARCHIVE_OK;
}


LOCAL const StreamOps g_FileStreamOps = {
    .open = FileStreamOpen,
    .read = FileStreamRead,
    .write = FileStreamWrite,
    .seek = FileStreamSeek,
    .tell = FileStreamTell,
    .close = FileStreamClose,
};
LOCAL const StreamOps g_FileHandleStreamOps = {
    .open = FileStreamOpenNull,
    .read = FileStreamRead,
    .write = FileStreamWrite,
    .seek = FileStreamSeek,
    .tell = FileStreamTell,
    .close = FileStreamCloseNull,
};

ARCHIVE_IMPL struct Stream *FileStreamCreate(const char *path)
{
    if (path == NULL) {
        return NULL;
    }
    FileStream *file = calloc(1, sizeof(FileStream));
    if (file == NULL) {
        ARCHIVE_ERR("FileStreamCreate fail!\n");
        return NULL;
    }

    SET_OBJ_MAGIC(&file->stream, STREAM_MAGIC_FILE);
    if (strncpy_s(file->path, PATH_MAX, path, strlen(path)) != EOK) {
        ARCHIVE_ERR("FileStream copy path fail!\n");
        free(file);
        return NULL;
    }
    file->stream.ops = g_FileStreamOps;
    file->handle = NULL;
    return (struct Stream *)file;
}

ARCHIVE_IMPL struct Stream *FileHandleStreamCreate(FILE *handle)
{
    FileStream *file = calloc(1, sizeof(FileStream));
    if (file == NULL) {
        ARCHIVE_ERR("FileStreamCreate fail!\n");
        return NULL;
    }
    SET_OBJ_MAGIC(&file->stream, STREAM_MAGIC_FILE);
    file->stream.ops = g_FileHandleStreamOps;
    file->handle = handle;
    return (struct Stream *)file;
}

ARCHIVE_IMPL int FileStreamSetPath(struct Stream *stream, const char *path)
{
    ASSERT_IF_MAGIC_ERR(stream, STREAM_MAGIC_FILE);
    if (stream == NULL || path == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    FileStream *file = (FileStream *)(void *)stream;
    if (strncpy_s(file->path, PATH_MAX, path, strlen(path)) != EOK) {
        ARCHIVE_ERR("FileStream copy path fail!\n");
        return ARCHIVE_INTERNAL_ERROR;
    }
    return ARCHIVE_OK;
}