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

#ifndef FILE_MAMAGEMENT_COMPRESS_OH_ARCHIVE_ERRCODE_H
#define FILE_MAMAGEMENT_COMPRESS_OH_ARCHIVE_ERRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OH_ARCHIVE_OK = 0,
    OH_ARCHIVE_PARAM_ERROR = 401,
    OH_ARCHIVE_UNKNOWN_ERROR = 13900100,
    OH_ARCHIVE_CANCEL_ERROR = 13900101,
    OH_ARCHIVE_UNSUPPORTED_ERROR = 13900102,
    OH_ARCHIVE_MEM_ERROR = 13900103,
    OH_ARCHIVE_OPEN_ERROR = 13900104,
    OH_ARCHIVE_WRITE_ERROR = 13900105,
    OH_ARCHIVE_READ_ERROR = 13900106,
    OH_ARCHIVE_STREAM_OUTPUT_ERROR = 13900107,
    OH_ARCHIVE_INSUFFICIENT_OUTBUF_ERROR = 13900108,
    OH_ARCHIVE_NO_SPACE_ERROR = 13900200,
    OH_ARCHIVE_PATH_NOT_EXIST_ERROR = 13900201,
    OH_ARCHIVE_PATH_EXISTS_ERROR = 13900202,
    OH_ARCHIVE_PATH_ACCESS_ERROR = 13900203,
    OH_ARCHIVE_NAME_TOO_LONG_ERROR = 13900204,
    OH_ARCHIVE_DATA_ERROR = 13900300,
    OH_ARCHIVE_CRC_ERROR = 13900301,
    OH_ARCHIVE_DEFLATE_ERROR = 13900302,
} OH_Archive_ErrCode;

#ifdef __cplusplus
}
#endif
#endif // FILE_MAMAGEMENT_COMPRESS_OH_ARCHIVE_ERRCODE_H