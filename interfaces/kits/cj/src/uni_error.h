/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILE_FS_UNI_ERROR_H
#define OHOS_FILE_FS_UNI_ERROR_H

#include "macro.h"
#include "uv.h"

#include <cstring>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

constexpr int UNKNOWN_ERR = -1;
constexpr int ERRNO_NOERR = 0;
constexpr int STORAGE_SERVICE_SYS_CAP_TAG = 13600000;
constexpr int FILEIO_SYS_CAP_TAG = 13900000;
constexpr int USER_FILE_MANAGER_SYS_CAP_TAG = 14000000;
constexpr int USER_FILE_SERVICE_SYS_CAP_TAG = 14300000;
constexpr int DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG = 22400000;
constexpr int SOFTBUS_TRANS_FILE_PERMISSION_DENIED = -426114938;
constexpr int SOFTBUS_TRANS_FILE_DISK_QUOTA_EXCEEDED = -426114937;
constexpr int SOFTBUS_TRANS_FILE_NO_MEMORY = -426114936;
constexpr int SOFTBUS_TRANS_FILE_NETWORK_ERROR = -426114935;
constexpr int SOFTBUS_TRANS_FILE_NOT_FOUND = -426114934;
constexpr int SOFTBUS_TRANS_FILE_EXISTED = -426114933;
constexpr int DFS_CANCEL_SUCCESS = 204;

enum ErrCodeSuffixOfFileIO {
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
    E_UKERR,
    E_NOLCK
};

enum ErrCodeSuffixOfUserFileManager {
    E_DISPLAYNAME = 1,
    E_URIM,
    E_SUFFIX,
    E_TRASH,
    E_OPEN_MODE,
    E_NOT_ALBUM,
    E_ROOT_DIR,
    E_MOVE_DENIED,
    E_RENAME_DENIED,
    E_RELATIVEPATH,
    E_INNER_FAIL,
    E_FILE_TYPE,
    E_FILE_KEY,
    E_INPUT
};

enum ErrCodeSuffixOfStorageService {
    E_IPCSS = 1,
    E_NOTSUPPORTEDFS,
    E_MOUNT,
    E_UNMOUNT,
    E_VOLUMESTATE,
    E_PREPARE,
    E_DELETE,
    E_NOOBJECT,
    E_OUTOFRANGE
};

enum ErrCodeSuffixOfUserFileService {
    E_IPCS = 1,
    E_URIS,
    E_GETINFO,
    E_GETRESULT,
    E_REGISTER,
    E_REMOVE,
    E_INIT,
    E_NOTIFY,
    E_CONNECT
};

enum ErrCodeSuffixOfDistributedFile {
    E_CLOUD_NOT_READY = 1,
    E_NETWORK_ERR,
    E_BATTERY_WARNING
};

enum CommonErrCode {
    E_PERMISSION = 201,
    E_PERMISSION_SYS = 202,
    E_PARAMS = 401,
    E_DEVICENOTSUPPORT = 801,
    E_OSNOTSUPPORT = 901
};

static inline std::unordered_map<int, int> softbusErr2ErrCodeTable {
    {SOFTBUS_TRANS_FILE_PERMISSION_DENIED, EPERM},
    {SOFTBUS_TRANS_FILE_DISK_QUOTA_EXCEEDED, EIO},
    {SOFTBUS_TRANS_FILE_NO_MEMORY, ENOMEM},
    {SOFTBUS_TRANS_FILE_NETWORK_ERROR, ENETUNREACH},
    {SOFTBUS_TRANS_FILE_NOT_FOUND, ENOENT},
    {SOFTBUS_TRANS_FILE_EXISTED, EEXIST},
    {DFS_CANCEL_SUCCESS, ECANCELED},
};

const std::unordered_map<std::string, int> uvCode2ErrCodeTable {
    { "EPERM", EPERM },
    { "ENOENT", ENOENT },
    { "ESRCH", ESRCH },
    { "EINTR", EINTR },
    { "EIO", EIO },
    { "ENXIO", ENXIO },
    { "E2BIG", E2BIG },
    { "EBADF", EBADF },
    { "ECHILD", ECHILD },
    { "EAGAIN", EAGAIN },
    { "ENOMEM", ENOMEM },
    { "EACCES", EACCES },
    { "EFAULT", EFAULT },
    { "EBUSY", EBUSY },
    { "EEXIST", EEXIST },
    { "EXDEV", EXDEV },
    { "ENODEV", ENODEV },
    { "ENOTDIR", ENOTDIR },
    { "EISDIR", EISDIR },
    { "EINVAL", EINVAL },
    { "ENFILE", ENFILE },
    { "EMFILE", EMFILE },
    { "ETXTBSY", ETXTBSY },
    { "EFBIG", EFBIG },
    { "ENOSPC", ENOSPC },
    { "ESPIPE", ESPIPE },
    { "EROFS", EROFS },
    { "EMLINK", EMLINK },
    { "EDEADLK", EDEADLK },
    { "ENAMETOOLONG", ENAMETOOLONG },
    { "ENOSYS", ENOSYS },
    { "ENOTEMPTY", ENOTEMPTY },
    { "ELOOP", ELOOP },
    { "EWOULDBLOCK", EWOULDBLOCK },
    { "EBADR", EBADR },
    { "ENOSTR", ENOSTR },
    { "ENODATA", ENODATA },
    { "EOVERFLOW", EOVERFLOW },
    { "EBADFD", EBADFD },
    { "ERESTART", ERESTART },
    { "EDQUOT", EDQUOT },
};

inline std::unordered_map<int, int32_t> errCodeTable {
    { EPERM, FILEIO_SYS_CAP_TAG + E_PERM},
    { ENOENT, FILEIO_SYS_CAP_TAG + E_NOENT},
    { ESRCH, FILEIO_SYS_CAP_TAG + E_SRCH},
    { EINTR, FILEIO_SYS_CAP_TAG + E_INTR},
    { EIO, FILEIO_SYS_CAP_TAG + E_IO},
    { ENXIO, FILEIO_SYS_CAP_TAG + E_NXIO},
    { E2BIG, FILEIO_SYS_CAP_TAG + E_2BIG},
    { EBADF, FILEIO_SYS_CAP_TAG + E_BADF},
    { ECHILD, FILEIO_SYS_CAP_TAG + E_CHILD},
    { EAGAIN, FILEIO_SYS_CAP_TAG + E_AGAIN},
    { ENOMEM, FILEIO_SYS_CAP_TAG + E_NOMEM},
    { EACCES, FILEIO_SYS_CAP_TAG + E_ACCES},
    { EFAULT, FILEIO_SYS_CAP_TAG + E_FAULT},
    { EBUSY, FILEIO_SYS_CAP_TAG + E_BUSY},
    { EEXIST, FILEIO_SYS_CAP_TAG + E_EXIST},
    { EXDEV, FILEIO_SYS_CAP_TAG + E_XDEV},
    { ENODEV, FILEIO_SYS_CAP_TAG + E_NODEV},
    { ENOTDIR, FILEIO_SYS_CAP_TAG + E_NOTDIR},
    { EISDIR, FILEIO_SYS_CAP_TAG + E_ISDIR},
    { EINVAL, FILEIO_SYS_CAP_TAG + E_INVAL},
    { ENFILE, FILEIO_SYS_CAP_TAG + E_NFILE},
    { EMFILE, FILEIO_SYS_CAP_TAG + E_MFILE},
    { ETXTBSY, FILEIO_SYS_CAP_TAG + E_TXTBSY},
    { EFBIG, FILEIO_SYS_CAP_TAG + E_FBIG},
    { ENOSPC, FILEIO_SYS_CAP_TAG + E_NOSPC},
    { ESPIPE, FILEIO_SYS_CAP_TAG + E_SPIPE},
    { EROFS, FILEIO_SYS_CAP_TAG + E_ROFS},
    { EMLINK, FILEIO_SYS_CAP_TAG + E_MLINK},
    { EDEADLK, FILEIO_SYS_CAP_TAG + E_DEADLK},
    { ENAMETOOLONG, FILEIO_SYS_CAP_TAG + E_NAMETOOLONG},
    { ENOSYS, FILEIO_SYS_CAP_TAG + E_NOSYS},
    { ENOTEMPTY, FILEIO_SYS_CAP_TAG + E_NOTEMPTY},
    { ELOOP, FILEIO_SYS_CAP_TAG + E_LOOP},
    { EWOULDBLOCK, FILEIO_SYS_CAP_TAG + E_WOULDBLOCK},
    { EBADR, FILEIO_SYS_CAP_TAG + E_BADR},
    { ENOSTR, FILEIO_SYS_CAP_TAG + E_NOSTR},
    { ENODATA, FILEIO_SYS_CAP_TAG + E_NODATA},
    { EOVERFLOW, FILEIO_SYS_CAP_TAG + E_OVERFLOW},
    { EBADFD, FILEIO_SYS_CAP_TAG + E_BADFD},
    { ERESTART, FILEIO_SYS_CAP_TAG + E_RESTART},
    { EDQUOT, FILEIO_SYS_CAP_TAG + E_DQUOT},
    { UNKNOWN_ERR, FILEIO_SYS_CAP_TAG + E_UKERR},
    { ENOLCK, FILEIO_SYS_CAP_TAG + E_NOLCK},
    { FILEIO_SYS_CAP_TAG + E_PERM, FILEIO_SYS_CAP_TAG + E_PERM},
    { FILEIO_SYS_CAP_TAG + E_NOENT, FILEIO_SYS_CAP_TAG + E_NOENT},
    { FILEIO_SYS_CAP_TAG + E_SRCH, FILEIO_SYS_CAP_TAG + E_SRCH},
    { FILEIO_SYS_CAP_TAG + E_INTR, FILEIO_SYS_CAP_TAG + E_INTR},
    { FILEIO_SYS_CAP_TAG + E_IO, FILEIO_SYS_CAP_TAG + E_IO},
    { FILEIO_SYS_CAP_TAG + E_NXIO, FILEIO_SYS_CAP_TAG + E_NXIO},
    { FILEIO_SYS_CAP_TAG + E_2BIG, FILEIO_SYS_CAP_TAG + E_2BIG},
    { FILEIO_SYS_CAP_TAG + E_BADF, FILEIO_SYS_CAP_TAG + E_BADF},
    { FILEIO_SYS_CAP_TAG + E_CHILD, FILEIO_SYS_CAP_TAG + E_CHILD},
    { FILEIO_SYS_CAP_TAG + E_AGAIN, FILEIO_SYS_CAP_TAG + E_AGAIN},
    { FILEIO_SYS_CAP_TAG + E_NOMEM, FILEIO_SYS_CAP_TAG + E_NOMEM},
    { FILEIO_SYS_CAP_TAG + E_ACCES, FILEIO_SYS_CAP_TAG + E_ACCES},
    { FILEIO_SYS_CAP_TAG + E_FAULT, FILEIO_SYS_CAP_TAG + E_FAULT},
    { FILEIO_SYS_CAP_TAG + E_BUSY, FILEIO_SYS_CAP_TAG + E_BUSY},
    { FILEIO_SYS_CAP_TAG + E_EXIST, FILEIO_SYS_CAP_TAG + E_EXIST},
    { FILEIO_SYS_CAP_TAG + E_XDEV, FILEIO_SYS_CAP_TAG + E_XDEV},
    { FILEIO_SYS_CAP_TAG + E_NODEV, FILEIO_SYS_CAP_TAG + E_NODEV},
    { FILEIO_SYS_CAP_TAG + E_NOTDIR, FILEIO_SYS_CAP_TAG + E_NOTDIR},
    { FILEIO_SYS_CAP_TAG + E_ISDIR, FILEIO_SYS_CAP_TAG + E_ISDIR},
    { FILEIO_SYS_CAP_TAG + E_INVAL, FILEIO_SYS_CAP_TAG + E_INVAL},
    { FILEIO_SYS_CAP_TAG + E_NFILE, FILEIO_SYS_CAP_TAG + E_NFILE},
    { FILEIO_SYS_CAP_TAG + E_MFILE, FILEIO_SYS_CAP_TAG + E_MFILE},
    { FILEIO_SYS_CAP_TAG + E_TXTBSY, FILEIO_SYS_CAP_TAG + E_TXTBSY},
    { FILEIO_SYS_CAP_TAG + E_FBIG, FILEIO_SYS_CAP_TAG + E_FBIG},
    { FILEIO_SYS_CAP_TAG + E_NOSPC, FILEIO_SYS_CAP_TAG + E_NOSPC},
    { FILEIO_SYS_CAP_TAG + E_SPIPE, FILEIO_SYS_CAP_TAG + E_SPIPE},
    { FILEIO_SYS_CAP_TAG + E_ROFS, FILEIO_SYS_CAP_TAG + E_ROFS},
    { FILEIO_SYS_CAP_TAG + E_MLINK, FILEIO_SYS_CAP_TAG + E_MLINK},
    { FILEIO_SYS_CAP_TAG + E_DEADLK, FILEIO_SYS_CAP_TAG + E_DEADLK},
    { FILEIO_SYS_CAP_TAG + E_NAMETOOLONG, FILEIO_SYS_CAP_TAG + E_NAMETOOLONG},
    { FILEIO_SYS_CAP_TAG + E_NOSYS, FILEIO_SYS_CAP_TAG + E_NOSYS},
    { FILEIO_SYS_CAP_TAG + E_NOTEMPTY, FILEIO_SYS_CAP_TAG + E_NOTEMPTY},
    { FILEIO_SYS_CAP_TAG + E_LOOP, FILEIO_SYS_CAP_TAG + E_LOOP},
    { FILEIO_SYS_CAP_TAG + E_WOULDBLOCK, FILEIO_SYS_CAP_TAG + E_WOULDBLOCK},
    { FILEIO_SYS_CAP_TAG + E_BADR, FILEIO_SYS_CAP_TAG + E_BADR},
    { FILEIO_SYS_CAP_TAG + E_NOSTR, FILEIO_SYS_CAP_TAG + E_NOSTR},
    { FILEIO_SYS_CAP_TAG + E_NODATA, FILEIO_SYS_CAP_TAG + E_NODATA},
    { FILEIO_SYS_CAP_TAG + E_OVERFLOW, FILEIO_SYS_CAP_TAG + E_OVERFLOW},
    { FILEIO_SYS_CAP_TAG + E_BADFD, FILEIO_SYS_CAP_TAG + E_BADFD},
    { FILEIO_SYS_CAP_TAG + E_RESTART, FILEIO_SYS_CAP_TAG + E_RESTART},
    { FILEIO_SYS_CAP_TAG + E_DQUOT, FILEIO_SYS_CAP_TAG + E_DQUOT},
    { FILEIO_SYS_CAP_TAG + E_UKERR, FILEIO_SYS_CAP_TAG + E_UKERR},
    { FILEIO_SYS_CAP_TAG + E_NOLCK, FILEIO_SYS_CAP_TAG + E_NOLCK},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_DISPLAYNAME, USER_FILE_MANAGER_SYS_CAP_TAG + E_DISPLAYNAME},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_URIM, USER_FILE_MANAGER_SYS_CAP_TAG + E_URIM},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_SUFFIX, USER_FILE_MANAGER_SYS_CAP_TAG + E_SUFFIX},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_TRASH, USER_FILE_MANAGER_SYS_CAP_TAG + E_TRASH},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_OPEN_MODE, USER_FILE_MANAGER_SYS_CAP_TAG + E_OPEN_MODE},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_NOT_ALBUM, USER_FILE_MANAGER_SYS_CAP_TAG + E_NOT_ALBUM},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_ROOT_DIR, USER_FILE_MANAGER_SYS_CAP_TAG + E_ROOT_DIR},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_MOVE_DENIED, USER_FILE_MANAGER_SYS_CAP_TAG + E_MOVE_DENIED},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_RENAME_DENIED, USER_FILE_MANAGER_SYS_CAP_TAG + E_RENAME_DENIED},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_RELATIVEPATH, USER_FILE_MANAGER_SYS_CAP_TAG + E_RELATIVEPATH},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_INNER_FAIL, USER_FILE_MANAGER_SYS_CAP_TAG + E_INNER_FAIL},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_TYPE, USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_TYPE},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_KEY, USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_KEY},
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_INPUT, USER_FILE_MANAGER_SYS_CAP_TAG + E_INPUT},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_IPCSS, STORAGE_SERVICE_SYS_CAP_TAG + E_IPCSS},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOTSUPPORTEDFS, STORAGE_SERVICE_SYS_CAP_TAG + E_NOTSUPPORTEDFS},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_MOUNT, STORAGE_SERVICE_SYS_CAP_TAG + E_MOUNT},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_UNMOUNT, STORAGE_SERVICE_SYS_CAP_TAG + E_UNMOUNT},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_VOLUMESTATE, STORAGE_SERVICE_SYS_CAP_TAG + E_VOLUMESTATE},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_PREPARE, STORAGE_SERVICE_SYS_CAP_TAG + E_PREPARE},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_DELETE, STORAGE_SERVICE_SYS_CAP_TAG + E_DELETE},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_OUTOFRANGE, STORAGE_SERVICE_SYS_CAP_TAG + E_OUTOFRANGE},
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_IPCS, USER_FILE_SERVICE_SYS_CAP_TAG + E_IPCS},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_URIS, USER_FILE_SERVICE_SYS_CAP_TAG + E_URIS},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETINFO, USER_FILE_SERVICE_SYS_CAP_TAG + E_GETINFO},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETRESULT, USER_FILE_SERVICE_SYS_CAP_TAG + E_GETRESULT},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_REGISTER, USER_FILE_SERVICE_SYS_CAP_TAG + E_REGISTER},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_REMOVE, USER_FILE_SERVICE_SYS_CAP_TAG + E_REMOVE},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_INIT, USER_FILE_SERVICE_SYS_CAP_TAG + E_INIT},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_NOTIFY, USER_FILE_SERVICE_SYS_CAP_TAG + E_NOTIFY},
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_CONNECT, USER_FILE_SERVICE_SYS_CAP_TAG + E_CONNECT},
    { E_PERMISSION, E_PERMISSION},
    { E_PERMISSION_SYS, E_PERMISSION_SYS},
    { E_PARAMS, E_PARAMS},
    { E_DEVICENOTSUPPORT, E_DEVICENOTSUPPORT},
    { E_OSNOTSUPPORT, E_OSNOTSUPPORT},
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_CLOUD_NOT_READY, DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_CLOUD_NOT_READY},
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_NETWORK_ERR, DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_NETWORK_ERR},
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_BATTERY_WARNING, DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_BATTERY_WARNING},
};

int ConvertUVCode2ErrCode(int errCode);

int GetErrorCode(int errCode);

}
}
}
#endif // OHOS_FILE_FS_UNI_ERROR_H