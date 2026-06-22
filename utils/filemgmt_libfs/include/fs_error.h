/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H
#define UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

#if (defined IOS_PLATFORM) || (defined WIN_PLATFORM)
constexpr int EBADR = 53;
constexpr int EBADFD = 77;
constexpr int ERESTART = 85;
#endif
#ifdef WIN_PLATFORM
constexpr int EDQUOT = 122;
#endif
constexpr int UNKNOWN_ERR = -1;
constexpr int NO_TASK_ERR = -2;
constexpr int CANCEL_ERR = -3;
constexpr int ERRNO_NOERR = 0;
constexpr int NO_ERR_MSG_ERR = -13999999;
constexpr int ECONNECTIONFAIL = 45;
constexpr int ECONNECTIONABORT = 46;
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
extern const std::string FILEIO_TAG_ERR_CODE;
extern const std::string FILEIO_TAG_ERR_DATA;

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
    E_NOLCK,
    E_NETUNREACH,
    E_CONNECTION_FAIL,
    E_CONNECTION_ABORT,
    E_NOTASK,
    E_UNCANCELED,
    E_CANCELED,
    E_INTERN_RES,
    E_MMAP_OOB,
    E_MMAP_FREE,
    E_MMAP_RO,
    E_MMAP_ACCS,
    E_MMAP_UNSUP,
    E_MMAP_FILE,
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
    E_CONNECT,
    E_CALLBACK_AND_URI_HAS_NOT_RELATIONS,
    E_CALLBACK_IS_NOT_REGISTER,
    E_CAN_NOT_FIND_URI,
    E_DO_NOT_HAVE_PARENT,
    E_LOAD_SA,
    E_COUNT
};

enum ErrCodeSuffixOfDistributedFile {
    E_CLOUD_NOT_READY = 1,
    E_NETWORK_ERR,
    E_BATTERY_WARNING,
    E_EXCEED_MAX_LIMIT,
    E_DATABASE_FAILED
};

enum CommonErrCode {
    E_PERMISSION = 201,
    E_PERMISSION_SYS = 202,
    E_PARAMS = 401,
    E_DEVICENOTSUPPORT = 801,
    E_OSNOTSUPPORT = 901,
    E_UNKNOWN_ERROR = 13900042
};

extern std::unordered_map<int, int> softbusErr2ErrCodeTable;

extern std::unordered_map<std::string_view, int> uvCode2ErrCodeTable;

extern std::unordered_map<int, std::pair<int32_t, std::string>> errCodeTable;

class FsError {
public:
    FsError(int errCode);
    FsError(int errCode, const std::string &errMsg);
    int GetErrNo() const;
    const std::string &GetErrMsg() const;
    ~FsError() = default;
    explicit operator bool() const;

private:
    int errno_ = ERRNO_NOERR;
    std::string errMsg_;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H
