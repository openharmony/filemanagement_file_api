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

#ifndef FILEMANAGEMENT_FILE_API_ERRORCODE_H
#define FILEMANAGEMENT_FILE_API_ERRORCODE_H

#define ARCHIVE_OK (0)
#define  ARCHIVE_PARAM_ERROR (-1)
#define  ARCHIVE_STREAM_ERROR (-2)
#define  ARCHIVE_DATA_ERROR (-3)
#define  ARCHIVE_MEM_ERROR (-4)
#define  ARCHIVE_BUF_ERROR (-5)
#define  ARCHIVE_VERSION_ERROR (-6)
#define  ARCHIVE_END_OF_LIST (-7)
#define  ARCHIVE_END_OF_STREAM (-8)
#define  ARCHIVE_FORMAT_ERROR (-9)
#define  ARCHIVE_INTERNAL_ERROR (-10)
#define  ARCHIVE_CRC_ERROR (-11)
#define  ARCHIVE_CRYPT_ERROR (-12)
#define  ARCHIVE_EXIST_ERROR (-13)
#define  ARCHIVE_PASSWORD_ERROR (-14)
#define  ARCHIVE_SUPPORT_ERROR (-15)
#define  ARCHIVE_HASH_ERROR (-16)
#define  ARCHIVE_OPEN_ERROR (-17)
#define  ARCHIVE_CLOSE_ERROR (-18)
#define  ARCHIVE_SEEK_ERROR (-19)
#define  ARCHIVE_TELL_ERROR (-20)
#define  ARCHIVE_READ_ERROR (-21)
#define  ARCHIVE_WRITE_ERROR (-22)
#define  ARCHIVE_SIGN_ERROR (-23)
#define  ARCHIVE_SYMLINK_ERROR (-24)
#define  ARCHIVE_MKDIR_ERROR (-25)
#define  ARCHIVE_CANCEL_ERROR (-26)
#define  ARCHIVE_NAME_TOO_LONG_ERROR (-27)
#define  ARCHIVE_NO_SPACE_ERROR (-28)
#define  ARCHIVE_FILE_NAME_TRUNCATED (-29)  // 文件名被截断
#define  ARCHIVE_FULL_PATH_TOO_LONG (-30)  // 全路径超长(PATH_MAX)
#define  ARCHIVE_EPERM_ERROR (-31)
#define  ARCHIVE_EIO_ERROR (-32)
#define  ARCHIVE_EACCES_ERROR (-33)
#define  ARCHIVE_ENOENT_ERROR (-34)

#define  ARCHIVE_ZLIB_ERROR (-37)

#endif //FILEMANAGEMENT_FILE_API_ERRORCODE_H