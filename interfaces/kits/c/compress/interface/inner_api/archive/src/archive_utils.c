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
#include <stdlib.h>

#include "archive_macros.h"
#include "securec.h"
#include "errorcode.h"
#include "archive_inner.h"

#define MKDIR(dir) mkdir(dir, 0755)
#define PATH_SEPARATOR '/'
#define MAX_RENAME_COUNT 1000

static bool IsDirectoryExists(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

static int ValidatePathPrefix(const char *path, const char *base)
{
    size_t baseLen = base == NULL ? 0 : strlen(base);
    if (baseLen == 0) {
        return ARCHIVE_OK;
    }

    if (strncmp(path, base, baseLen) != 0 ||
        (base[baseLen - 1] != '/' && path[baseLen] != '/' && path[baseLen] != '\0')) {
        return ARCHIVE_PARAM_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int CreateDirectory(const char *path, const char *base)
{
    if (IsDirectoryExists(path)) {
        return ARCHIVE_OK;
    }

    int ret = ValidatePathPrefix(path, base);
    if (ret != ARCHIVE_OK) {
        return ret;
    }

    size_t baseLen = base == NULL ? 0 : strlen(base);
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

ARCHIVE_IMPL int CreateSymlink(const char *old, const char *newSym)
{
    if (symlink(old, newSym) != 0) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    return ARCHIVE_OK;
}

ARCHIVE_IMPL int CreateParentDirectory(const char *filePath, const char *base)
{
    char parentPath[ZIP_FILE_NAME_LEN_MAX];
    const char *lastSeparator = NULL;
    const char *p;

    if (!filePath || strlen(filePath) >= ZIP_FILE_NAME_LEN_MAX) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    for (p = filePath; *p; p++) {
        if (*p == '/') {
            lastSeparator = p;
        }
    }

    if (lastSeparator == NULL) {
        return ARCHIVE_OK;
    }

    size_t parentPathLen = lastSeparator - filePath;
    if (parentPathLen >= ZIP_FILE_NAME_LEN_MAX) {
        return ARCHIVE_FULL_PATH_TOO_LONG;
    }

    int ret = strncpy_s(parentPath, ZIP_FILE_NAME_LEN_MAX, filePath, parentPathLen);
    if (ret != EOK) {
        return ARCHIVE_INTERNAL_ERROR;
    }
    parentPath[parentPathLen] = '\0';

    if (parentPath[0] == '\0') {
        return ARCHIVE_OK;
    }

    return CreateDirectory(parentPath, base);
}

static int32_t PathHandleDot(const char **source, char **target, int32_t *maxOutput,
                             const char *path, char *output, const char *check)
{
    check += 1;

    if ((*check == 0) && (*source != path && *target != output)) {
        **target = **source;
        (*target)++;
        (*maxOutput)--;
        *source += (check - *source);
        return 1;
    } else if (*check == '/') {
        *source += (check - *source);
        if (**source != 0 && *target == output) {
            (*source)++;
        }
        return 1;
    }

    return 0;
}

static void PathHandleDotDotSearchBack(char **target, int32_t *maxOutput, char *output)
{
    if (*target != output) {
        (*target)--;
        do {
            if ((*target) == output || **target == '/') {
                break;
            }

            (*target)--;
            (*maxOutput)++;
        } while (*target > output);
    }
}

static int32_t PathHandleDotDot(const char **source, char **target, int32_t *maxOutput,
                                const char *path, char *output, const char *check)
{
    check += 2; // skip 2 chars ".."

    if ((*check == 0) || (*check == '/')) {
        *source += (check - *source);
        PathHandleDotDotSearchBack(target, maxOutput, output);
        if (**source != 0 && *target == output) {
            (*source)++;
        }

        if (**target == '/' && **source == 0) {
            (*target)++;
        }

        **target = 0;
        return 1;
    }
    return 0;
}

static int32_t NormalizePathDot(const char **source, char **target, int32_t *maxOutput,
    const char *path, char *output, const char *check)
{
    if (*check == '.') {
        if (PathHandleDot(source, target, maxOutput, path, output, check)) {
            return 1;
        }

        if (*(check + 1) == '.' && PathHandleDotDot(source, target, maxOutput, path,
            output, check)) {
            return 1;
        }
    }

    return 0;
}

static char *NormalizePath(const char *path)
{
    int32_t maxOutputLen = strlen(path) + 1;
    char *output = (char *)calloc(maxOutputLen, sizeof(char));
    if (output == NULL) {
        return NULL;
    }

    const char *source = path;
    const char *check = output;
    char *target = output;
    while (*source != 0 && maxOutputLen > 1) {
        check = source;
        if (*check == '/') {
            check++;
        }

        if ((source == path) || (target == output) || (check != source)) {
            if (*check == '/') {
                source++;
                continue;
            }

            if (NormalizePathDot(&source, &target, &maxOutputLen, path, output, check)) {
                continue;
            }
        }

        *target = *source;
        source++;
        target++;
        maxOutputLen--;
    }
    *target = 0;

    if (*path == 0) {
        free(output);
        return NULL;
    }

    return output;
}

ARCHIVE_IMPL int GetOutputFilePath(const char *fileName, const char *outDir, char *outPath,
    size_t size)
{
    char *normalizedPath = NormalizePath(fileName);
    if (normalizedPath == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    char completePath[ZIP_FILE_NAME_LEN_MAX];
    int ret = snprintf_s(completePath, ZIP_FILE_NAME_LEN_MAX, ZIP_FILE_NAME_LEN_MAX - 1,
        "%s/%s", outDir, normalizedPath);
    free(normalizedPath);
    if (ret < 0) {
        return ARCHIVE_FULL_PATH_TOO_LONG;
    }

    char *completePathNormalized = NormalizePath(completePath);
    if (completePathNormalized == NULL) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    ret = snprintf_s(outPath, size, size - 1, "%s", completePathNormalized);
    free(completePathNormalized);
    if (ret < 0) {
        return ARCHIVE_INTERNAL_ERROR;
    }

    return ARCHIVE_OK;
}

#ifndef EMPTY_REPLACE_NAME
#define EMPTY_REPLACE_NAME "_"
#endif

typedef struct SegNode {
    char *seg;
    struct SegNode *next;
} SegNode;

static int FileNameCompare(const char *fileName1, const char *fileName2, uint8_t ignoreCase);

static int FileNameComprareStar(const char *fileName1, const char *fileName2, uint8_t ignoreCase)
{
    if (*(fileName2 + 1) == 0) {
        return ARCHIVE_OK;
    }

    while (*fileName1 != 0) {
        if (FileNameCompare(fileName1, fileName2 + 1, ignoreCase) == ARCHIVE_OK) {
            return ARCHIVE_OK;
        }
        fileName1++;
    }

    return ARCHIVE_EXIST_ERROR;
}

static int FileNameCompareDefault(const char *fileName1, const char *fileName2, uint8_t ignoreCase)
{
    if (ignoreCase) {
        if (tolower(*fileName1) != tolower(*fileName2)) {
            return ARCHIVE_EXIST_ERROR;
        }
    } else {
        if (*fileName1 != *fileName2) {
            return ARCHIVE_EXIST_ERROR;
        }
    }

    return ARCHIVE_OK;
}

static int FileNameCompare(const char *fileName1, const char *fileName2, uint8_t ignoreCase)
{
    int ret;
    while (*fileName1 != 0) {
        switch (*fileName2) {
            case '*':
                return FileNameComprareStar(fileName1, fileName2, ignoreCase);
            default:
                ret = FileNameCompareDefault(fileName1, fileName2, ignoreCase);
                if (ret != ARCHIVE_OK) {
                    return ret;
                }
                break;
        }
        fileName1++;
        fileName2++;
    }

    if (*fileName2 != 0 && *fileName2 != '*') {
        return ARCHIVE_EXIST_ERROR;
    }

    return ARCHIVE_OK;
}

ARCHIVE_IMPL int IsFileExists(const char *fileName)
{
    struct stat path;
    if (stat(fileName, &path) == 0) {
        return ARCHIVE_OK;
    }
    return ARCHIVE_EXIST_ERROR;
}

ARCHIVE_IMPL int IsSymlinkExists(const char *fileName)
{
    struct stat path;
    if (lstat(fileName, &path) == -1) {
        return ARCHIVE_EXIST_ERROR;
    }

    return S_ISLNK(path.st_mode) ? ARCHIVE_OK : ARCHIVE_EXIST_ERROR;
}

static int TryNewFileNameCount(const char *dir, const char *base, const char *ext,
    char *newPath, size_t newPathSize)
{
    int ret;
    int counter = 2;
    char tempPath[ZIP_FILE_NAME_LEN_MAX];

    while (1) {
        if (dir[0] == '.' && dir[1] == '/') {
            ret = snprintf_s(tempPath, sizeof(tempPath), sizeof(tempPath) - 1, "%s%s (%d)%s",
                dir + 2, base, counter, ext);   // skip "./", 2 is length of "./"
            if (ret < 0) {
                return ARCHIVE_EXIST_ERROR;
            }
        } else {
            ret = snprintf_s(tempPath, sizeof(tempPath), sizeof(tempPath) - 1, "%s%s (%d)%s",
                dir, base, counter, ext);
            if (ret < 0) {
                return ARCHIVE_EXIST_ERROR;
            }
        }

        if (access(tempPath, F_OK) != 0) {
            ret = strncpy_s(newPath, newPathSize, tempPath, newPathSize);
            if (ret != EOK) {
                return ARCHIVE_EXIST_ERROR;
            }
            newPath[newPathSize - 1] = '\0';
            return ARCHIVE_OK;
        }
        counter++;

        if (counter >= MAX_RENAME_COUNT) {
            return ARCHIVE_EXIST_ERROR;
        }
    }
}

ARCHIVE_IMPL int GenerateNewFilename(const char *originalPath, char *newPath, size_t newPathSize)
{
    if (!originalPath || !newPath || newPathSize == 0) {
        return ARCHIVE_PARAM_ERROR;
    }
    int ret;
    char base[ZIP_FILE_NAME_LEN_MAX] = {0};
    char ext[ZIP_FILE_NAME_LEN_MAX] = {0};
    char dir[ZIP_FILE_NAME_LEN_MAX] = {0};

    const char *filename = originalPath;
    const char *lastSlash = strrchr(originalPath, '/');

    if (lastSlash) {
        filename = lastSlash + 1;
        size_t dirLen = lastSlash - originalPath + 1;
        if (dirLen > 1) {
            ret = strncpy_s(dir, ZIP_FILE_NAME_LEN_MAX, originalPath, dirLen);
            if (ret != EOK) {
                return ARCHIVE_EXIST_ERROR;
            }
            dir[dirLen] = '\0';
        } else if (dirLen == 1) {
            (void)strcpy_s(dir, ZIP_FILE_NAME_LEN_MAX, "/");
        } else {
            (void)strcpy_s(dir, ZIP_FILE_NAME_LEN_MAX, "./");
        }
    } else {
        (void)strcpy_s(dir, ZIP_FILE_NAME_LEN_MAX, "./");
    }

    const char *lastDot = strrchr(filename, '.');
    if (lastDot) {
        size_t baseLen = lastDot - filename;
        ret = strncpy_s(base, ZIP_FILE_NAME_LEN_MAX, filename, baseLen);
        if (ret != EOK) {
            return ARCHIVE_EXIST_ERROR;
        }
        base[baseLen] = '\0';
        ret = strcpy_s(ext, ZIP_FILE_NAME_LEN_MAX, lastDot);
        if (ret != EOK) {
            return ARCHIVE_EXIST_ERROR;
        }
    } else {
        ret = strcpy_s(base, ZIP_FILE_NAME_LEN_MAX, filename);
        if (ret != EOK) {
            return ARCHIVE_EXIST_ERROR;
        }
        ext[0] = '\0';
    }
    return TryNewFileNameCount(dir, base, ext, newPath, newPathSize);
}

ARCHIVE_IMPL char *ConvertStrGBKToUTF8(const char *str)
{
    iconv_t cd;
    size_t result = 0;
    size_t strLength = 0;
    size_t strLengthUTF8 = 0;
    char *strUTF8 = NULL;
    char *strUTF8Ptr = NULL;

    if (!str) {
        return NULL;
    }

    cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        return NULL;
    }

    strLength = strlen(str);
    strLengthUTF8 = strLength * 2;  // max 2 bytes for each character
    strUTF8 = (char *)calloc((int32_t)(strLengthUTF8 + 1), sizeof(char));
    strUTF8Ptr = strUTF8;

    if (strUTF8) {
        result = iconv(cd, (char **)&str, &strLength,
                       (char **)&strUTF8Ptr, &strLengthUTF8);
    }

    iconv_close(cd);

    if (result == (size_t)-1) {
        free(strUTF8);
        strUTF8 = NULL;
    }

    return strUTF8;
}

static uint8_t CheckUTF8Byte(unsigned char ch, int32_t *nBytes)
{
    if ((*nBytes) != 0) {
        if ((ch & 0xc0) != 0x80) {
            return 0;
        }
        (*nBytes)--;
        return 1;
    }

    if ((ch & 0x80) != 0) {
        while ((ch & 0x80) != 0) {
            ch <<= 1;
            (*nBytes)++;
        }
        // UTF-8 has 2 to 6 bytes for each character
        if (((*nBytes) < 2) || ((*nBytes) > 6)) {
            return 0;
        }
        (*nBytes)--;
    }
    return 1;
}

static uint8_t CheckUTF8Str(const char *str, size_t length)
{
    size_t index = 0;
    int32_t nBytes = 0;
    while (index < length) {
        if (!CheckUTF8Byte(str[index], &nBytes)) {
            return 0;
        }
        index++;
    }
    return (nBytes == 0);
}

ARCHIVE_IMPL uint8_t IsUTF8Str(char *fStr)
{
    return CheckUTF8Str(fStr, strlen(fStr));
}

bool IsGBKEncoded(const unsigned char *str)
{
    if (str == NULL) {
        return false;
    }

    while (*str) {
        if (*str <= 0x7F) {
            str++;
        } else if (*str >= 0x81 && *str <= 0xFE) {
            str++;
            if (*str == '\0') {
                return false;
            }

            if ((*str >= 0x40 && *str <= 0x7E) || (*str >= 0x80 && *str <= 0xFE)) {
                str++;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

static size_t Utf8PreNum(unsigned char byte)
{
    unsigned char mask = 0x80;
    size_t num = 0;
    for (int i = 0; i < 8; i++) {   // traverse 8 bits
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
    return num;
}

bool IsUtf8Encoded(unsigned char *data, bool skipTwoBytes)
{
    size_t len = strlen((char *)data);
    size_t num = 0;
    size_t i = 0;
    size_t minPreNum = skipTwoBytes ? 2 : 1;
    while (i < len) {
        if ((data[i] & 0x80) == 0x00) {
            i++;
            continue;
        }
        num = Utf8PreNum(data[i]);
        if (num <= minPreNum) {
            return false;
        }
        if (num > len - i) {
            return false;
        }
        for (size_t j = 1; j < num; j++) {
            if ((data[i + j] & 0xc0) != 0x80) {
                return false;
            }
        }
        i += num;
    }
    return true;
}