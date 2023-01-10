/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_UNI_ERROR_H
#define INTERFACES_KITS_JS_SRC_COMMON_UNI_ERROR_H

#include <string>
#include <unordered_map>

#include "napi/uni_header.h"

namespace OHOS {
namespace DistributedFS {
constexpr int ERRNO_NOERR = 0;
constexpr int FILEIO_SYS_CAP_TAG = 13900000;
const std::string FILEIO_TAG_ERR_CODE = "code";
const std::string FILEIO_TAG_ERR_MSG = "message";

enum ELegacy {
    ELEGACY_INVAL = 202,
    ELEGACY_IO = 300,
    ELEGACY_NOENT = 301,
};

enum ErrCodeSystem {
    ERR_CODE_SYSTEM_LEGACY,
    ERR_CODE_SYSTEM_POSIX,
};

enum ErrCodeSuffix {
    E_PERM = 1,
    E_NOENT,
    E_SRCH,
    E_INTR,
    E_IO,
    E_NXIO,
    E_2BIG,
    E_BADF,
    E_CHILD,
    E_AGAIN,
    E_NOMEM,
    E_ACCES,
    E_FAULT,
    E_BUSY,
    E_EXIST,
    E_XDEV,
    E_NODEV,
    E_NOTDIR,
    E_ISDIR,
    E_INVAL,
    E_NFILE,
    E_MFILE,
    E_TXTBSY,
    E_FBIG,
    E_NOSPC,
    E_SPIPE,
    E_ROFS,
    E_MLINK,
    E_DEADLK,
    E_NAMETOOLONG,
    E_NOSYS,
    E_NOTEMPTY,
    E_LOOP,
    E_WOULDBLOCK,
    E_BADR,
    E_NOSTR,
    E_NODATA,
    E_OVERFLOW,
    E_BADFD,
    E_RESTART,
    E_DQUOT,
    E_UKERR
};

const std::unordered_map<int, std::pair<int32_t, std::string>> errCodeTable {
    { EPERM, { FILEIO_SYS_CAP_TAG + E_PERM, "Operation not permitted" } },
    { ENOENT, { FILEIO_SYS_CAP_TAG + E_NOENT, "No such file or directory" } },
    { ESRCH, { FILEIO_SYS_CAP_TAG + E_SRCH, "No such process" } },
    { EINTR, { FILEIO_SYS_CAP_TAG + E_INTR, "Interrupted system call" } },
    { EIO, { FILEIO_SYS_CAP_TAG + E_IO, "I/O error" } },
    { ENXIO, { FILEIO_SYS_CAP_TAG + E_NXIO, "No such device or address" } },
    { E2BIG, { FILEIO_SYS_CAP_TAG + E_2BIG, "Arg list too long" } },
    { EBADF, { FILEIO_SYS_CAP_TAG + E_BADF, "Bad file descriptor" } },
    { ECHILD, { FILEIO_SYS_CAP_TAG + E_CHILD, "No child processes" } },
    { EAGAIN, { FILEIO_SYS_CAP_TAG + E_AGAIN, "Try again" } },
    { ENOMEM, { FILEIO_SYS_CAP_TAG + E_NOMEM, "Out of memory" } },
    { EACCES, { FILEIO_SYS_CAP_TAG + E_ACCES, "Permission denied" } },
    { EFAULT, { FILEIO_SYS_CAP_TAG + E_FAULT, "Bad address" } },
    { EBUSY, { FILEIO_SYS_CAP_TAG + E_BUSY, "Device or resource busy" } },
    { EEXIST, { FILEIO_SYS_CAP_TAG + E_EXIST, "File exists" } },
    { EXDEV, { FILEIO_SYS_CAP_TAG + E_XDEV, "Cross-device link" } },
    { ENODEV, { FILEIO_SYS_CAP_TAG + E_NODEV, "No such device" } },
    { ENOTDIR, { FILEIO_SYS_CAP_TAG + E_NOTDIR, "Not a directory" } },
    { EISDIR, { FILEIO_SYS_CAP_TAG + E_ISDIR, "Is a directory" } },
    { EINVAL, { FILEIO_SYS_CAP_TAG + E_INVAL, "Invalid argument" } },
    { ENFILE, { FILEIO_SYS_CAP_TAG + E_NFILE, "File table overflow" } },
    { EMFILE, { FILEIO_SYS_CAP_TAG + E_MFILE, "Too many open files" } },
    { ETXTBSY, { FILEIO_SYS_CAP_TAG + E_TXTBSY, "Text file busy" } },
    { EFBIG, { FILEIO_SYS_CAP_TAG + E_FBIG, "File too large" } },
    { ENOSPC, { FILEIO_SYS_CAP_TAG + E_NOSPC, "No space left on device" } },
    { ESPIPE, { FILEIO_SYS_CAP_TAG + E_SPIPE, "Illegal seek" } },
    { EROFS, { FILEIO_SYS_CAP_TAG + E_ROFS, "Read-only file system" } },
    { EMLINK, { FILEIO_SYS_CAP_TAG + E_MLINK, "Too many links" } },
    { EDEADLK, { FILEIO_SYS_CAP_TAG + E_DEADLK, "Resource deadlock would occur" } },
    { ENAMETOOLONG, { FILEIO_SYS_CAP_TAG + E_NAMETOOLONG, "File name too long" } },
    { ENOSYS, { FILEIO_SYS_CAP_TAG + E_NOSYS, "Function not implemented" } },
    { ENOTEMPTY, { FILEIO_SYS_CAP_TAG + E_NOTEMPTY, "Directory not empty" } },
    { ELOOP, { FILEIO_SYS_CAP_TAG + E_LOOP, "Too many symbolic links encountered" } },
    { EWOULDBLOCK, { FILEIO_SYS_CAP_TAG + E_WOULDBLOCK, "Operation would block" } },
    { EBADR, { FILEIO_SYS_CAP_TAG + E_BADR, "Invalid request descriptor" } },
    { ENOSTR, { FILEIO_SYS_CAP_TAG + E_NOSTR, "Device not a stream" } },
    { ENODATA, { FILEIO_SYS_CAP_TAG + E_NODATA, "No data available" } },
    { EOVERFLOW, { FILEIO_SYS_CAP_TAG + E_OVERFLOW, "Value too large for defined data type" } },
    { EBADFD, { FILEIO_SYS_CAP_TAG + E_BADFD, "File descriptor in bad state" } },
    { ERESTART, { FILEIO_SYS_CAP_TAG + E_RESTART, "Interrupted system call should be restarted" } },
    { EDQUOT, { FILEIO_SYS_CAP_TAG + E_DQUOT, "Quota exceeded" } },
    { -1, { FILEIO_SYS_CAP_TAG + E_UKERR, "Unknown error" } }
};

class UniError {
public:
    UniError();
    explicit UniError(ELegacy eLegacy);
    explicit UniError(int ePosix);
    explicit UniError(int ePosix, bool throwCode);
    UniError(const UniError &) = default;
    ~UniError() = default;

    UniError &operator = (const UniError &) = default;

    explicit operator bool() const;

    void SetErrno(ELegacy eLegacy);
    void SetErrno(int ePosix);
    int GetErrno(ErrCodeSystem cs);

    std::string GetDefaultErrstr();
    napi_value GetNapiErr(napi_env env);
    napi_value GetNapiErr(napi_env env, std::string errMsg);
    void ThrowErr(napi_env env);
    void ThrowErr(napi_env env, int code);
    void ThrowErr(napi_env env, std::string errMsg);

private:
    int errno_ = ERRNO_NOERR;
    ErrCodeSystem codingSystem_ = ERR_CODE_SYSTEM_POSIX;
    bool throwCode_ = false;
};
} // namespace DistributedFS
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_COMMON_UNI_ERROR_H