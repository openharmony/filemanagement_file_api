/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef RUST_FILE_H
#define RUST_FILE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @ingroup     rust
 * @brief       Enumeration of `lseek` interface to seek within a file.
 * @param       Start   Sets the offset from the head of the file.
 * @param       Current Sets the offset from the current position.
 * @param       End     Sets the offset from th tail of the file.
 */
enum SeekPos {
    START,
    CURRENT,
    END
};

/**
 * @ingroup     rust
 * @brief       Enumeration of `mkdirs` interface to choose ways to create the direction.
 * @param       Single   Creates a single level directory.
 * @param       Multiple Creates a multi-level directory.
 */
enum MakeDirectionMode {
    SINGLE,
    MULTIPLE
};

/**
 * @ingroup     rust
 * @struct      Str
 * @brief       Stores string and its effective length.
 */
typedef struct {
    const char* str;
    unsigned int len;
} Str;

/**
 * @ingroup     rust
 * @brief       Gets a iterator to read the content of a file in a path and split it by line.
 * @param       path    file path.
 * @retval      NULL    Fails to read the file, stores error information from errno.
 * @retval      !NULL   Reads the file successfully, valid reader iterator pointer.
 * @attention
 * 1.The return value needs to be released by calling the `stringVectorFree` method.
 * 2.The input `path` must be in valid UTF-8 format.
 * 3.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
void* ReaderIterator(const char* path);

/**
 * @ingroup     rust
 * @brief       Reads a line from the reader iterator.
 * @param       iter   pointer to reader iterator.
 * @retval      NULL and error stored in errno  Invalid pointer to iterator.
 * @retval      NULL and no error in errno      Gets the last line of content from the iterator.
 * @retval      !NULL                           Valid `Str` pointer, Gets a line of content successfully.
 * @attention
 * 1.The input `lines` must be a valid pointer to `StringVector`.
 * 2.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
Str* NextLine(void* iter);

/**
 * @ingroup     rust
 * @brief       Seeks to an offset, in bytes, in a file.
 * @param       fd      file descriptor.
 * @param       offset  seek offset.
 * @param       pos     seek position.
 * @retval      >=0     the resulting offset location.
 * @retval      -1      error occurs.
 * @attention
 * 1.It can fail because it may involve flushing a buffer or seek to a negative offset.
 * 2.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
long long int Lseek(int fd, long long offset, enum SeekPos pos);

/**
 * @ingroup     rust
 * @brief       Creates a new directory at the given path.
 * @param       path    direction path.
 * @param       mode    creating ways.
 * @retval      0       Created successfully.
 * @retval      -1      Creation failed.
 * @attention
 * 1.The input `path` must be in valid UTF-8 format.
 * 2.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
int Mkdirs(const char* path, enum MakeDirectionMode mode);

/**
 * @ingroup     rust
 * @brief       Get the parent directory of the specified file.
 * @param       fd      file descriptor.
 * @retval      NULL    The path terminates in a root or prefix or the file is closed.
 * @retval      !NULL   The parent directory of the specified file.
 * @attention
 * 1.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
Str* GetParent(int fd);

/**
 * @ingroup     rust
 * @brief       Cut the file name by the specified length.
 * @param       path    file name.
 * @param       size    specified length.
 * @retval      NULL    The file name is empty or error.
 * @retval      !NULL   The result of the file name after cutting.
 * @attention
 * 1.Errors are stored in errno.
 * @li          rust_file.h:The file where the interface is located.
 */
Str* CutFileName(const char* path, size_t size);

/**
 * @ingroup     rust
 * @brief       Releases the memory that the `Str` pointer points to.
 * @param       lines   pointer to `Str`.
 * @li          rust_file.h:The file where the interface is located.
 */
void StrFree(Str* str);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //RUST_FILE_H