/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
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

/**
 * @addtogroup Archive
 *
 * @brief Provides the definition of Archive error codes.
 * @{
 * @since 26.0.0
 */

/**
 * @file oh_archive_errcode.h
 *
 * @brief Declares the error codes of Archive module.
 * @kit CoreFileKit
 * @library liboharchive.so
 * @syscap SystemCapability.FileManagement.File.FileIO
 * @since 26.0.0
 */
#ifndef FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_ERRCODE_H
#define FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_ERRCODE_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Error codes for the Archive.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief Operation completed successfully.
     * @since 26.0.0
     */
    OH_ARCHIVE_OK = 0,
    /**
     * @brief Invalid input parameters.
     * @since 26.0.0
     */
    OH_ARCHIVE_PARAM_ERROR = 401,
    /**
     * @brief An unknown error occurred.
     * @since 26.0.0
     */
    OH_ARCHIVE_UNKNOWN_ERROR = 13900100,
    /**
     * @brief Operation cancelled by user.
     * @since 26.0.0
     */
    OH_ARCHIVE_CANCEL_ERROR = 13900101,
    /**
     * @brief Format not supported.
     * @since 26.0.0
     */
    OH_ARCHIVE_UNSUPPORTED_ERROR = 13900102,
    /**
     * @brief Memory allocation failed.
     * @since 26.0.0
     */
    OH_ARCHIVE_MEM_ERROR = 13900103,
    /**
     * @brief Failed to open archive file.
     * @since 26.0.0
     */
    OH_ARCHIVE_OPEN_ERROR = 13900104,
    /**
     * @brief Write operation failed.
     * @since 26.0.0
     */
    OH_ARCHIVE_WRITE_ERROR = 13900105,
    /**
     * @brief Read operation failed.
     * @since 26.0.0
     */
    OH_ARCHIVE_READ_ERROR = 13900106,
    /**
     * @brief Stream output error.
     * @since 26.0.0
     */
    OH_ARCHIVE_STREAM_OUTPUT_ERROR = 13900107,
    /**
     * @brief Insufficient output buffer error.
     * @since 26.0.0
     */
    OH_ARCHIVE_INSUFFICIENT_OUTBUF_ERROR = 13900108,
    /**
     * @brief Insufficient disk space.
     * @since 26.0.0
     */
    OH_ARCHIVE_NO_SPACE_ERROR = 13900200,
    /**
     * @brief Path does not exist.
     * @since 26.0.0
     */
    OH_ARCHIVE_PATH_NOT_EXIST_ERROR = 13900201,
    /**
     * @brief Path already exists.
     * @since 26.0.0
     */
    OH_ARCHIVE_PATH_EXISTS_ERROR = 13900202,
    /**
     * @brief Path access error.
     * @since 26.0.0
     */
    OH_ARCHIVE_PATH_ACCESS_ERROR = 13900203,
    /**
     * @brief File name too long.
     * @since 26.0.0
     */
    OH_ARCHIVE_NAME_TOO_LONG_ERROR = 13900204,
    /**
     * @brief Full path too long.
     * @since 26.0.0
     */
    OH_ARCHIVE_FULL_PATH_TOO_LONG_ERROR = 13900205,
    /**
     * @brief Data integrity error.
     * @since 26.0.0
     */
    OH_ARCHIVE_DATA_ERROR = 13900300,
    /**
     * @brief CRC check error.
     * @since 26.0.0
     */
    OH_ARCHIVE_CRC_ERROR = 13900301,
    /**
     * @brief Deflate method error.
     * @since 26.0.0
     */
    OH_ARCHIVE_DEFLATE_ERROR = 13900302
} OH_Archive_ErrCode;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
/** @} */
#endif /* FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_ERRCODE_H */