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
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <iconv.h>
#include "archive_macros.h"
#include "securec.h"
#include "errorcode.h"
#include "archive_inner.h"

#define MKDIR(dir) mkdir(dir, 0755)
#define PATH_SEPARATOR '/'

static bool IsDirectoryExists(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

ARCHIVE_IMPL int CreateDirectory(const char *path, const char *base)
{
    if (IsDirectoryExists(path)) {
        return ARCHIVE_OK;
    }

    size_t baseLen = base == NULL ? 0 : strlen(base);
    if (base != NULL) {
        if (strncmp(path, base, baseLen) != 0 ||
            (base[baseLen - 1] != '/' && path[baseLen] != '/' && path[baseLen] != '\0')) {
            return ARCHIVE_PARAM_ERROR;
        }
    }

    char temp[ZIP_FILE_NAME_LEN_MAX];
    const char *p = path;
    char *t = temp;
    size_t len = 0;

    if (*p == '/') {
        *t++ = *p++;
        len = 1;
    }

    while (*p) {
        while (*p && *p != '/') {
            if (len >= ZIP_FILE_NAME_LEN_MAX - 1) {
                return ARCHIVE_NAME_TOO_LONG_ERROR;
            }
            temp[len++] = *p++;
        }
        temp[len] = '\0';

        if (*p) {
            if (len >= ZIP_FILE_NAME_LEN_MAX - 1) {
                return ARCHIVE_NAME_TOO_LONG_ERROR;
            }
            temp[len++] = PATH_SEPARATOR;
            temp[len] = '\0';
            p++;
        }

        if (len > baseLen + 1) {
            if (!IsDirectoryExists(temp) && MKDIR(temp) != 0 && errno != EEXIST) {
                return errno == ENAMETOOLONG ? ARCHIVE_NAME_TOO_LONG_ERROR : ARCHIVE_PARAM_ERROR;
            }
        }
    }

    return ARCHIVE_OK;
}