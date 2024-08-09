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

#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "uni_error.h"
#include "uri.h"
#include "open.h"
#include "datashare_helper.h"
#include "utils.h"
#include "iremote_stub.h"
#include "file_uri.h"
#include "want.h"
#include "ability_manager_client.h"
#include "remote_uri.h"
#include "file_utils.h"
#include "securec.h"
#include "file_impl.h"

static const std::string PROCEDURE_OPEN_NAME = "FileIOOpen";
static const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
static const std::string FILE_DATA_URI = "file://";
static const std::string PATH_SHARE = "/data/storage/el2/share";
static const std::string MODE_RW = "/rw/";
static const std::string MODE_R = "/r/";
static const std::string DOCS = "docs";
static const std::string DATASHARE = "datashare";
static const std::string SCHEME_BROKER = "content";
constexpr uint32_t MAX_WANT_FLAG = 4;

namespace {
using Uri = OHOS::Uri;
using namespace OHOS;
using namespace DistributedFS;
using namespace OHOS::CJSystemapi;
using namespace OHOS::CJSystemapi::FileFs;
using namespace OHOS::FileManagement;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::DistributedFS::ModuleRemoteUri;
using namespace std;

static int OpenFileByPath(const std::string &path, unsigned int mode)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> open_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!open_req) {
        LOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        LOGE("Failed to open OpenFileByPath error %{public}d", ret);
    }
    return ret;
}

static int OpenFileByDatashare(const std::string &path, int64_t flags)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    sptr<FileIoToken> remote = new (std::nothrow) OHOS::IRemoteStub<FileIoToken>();
    if (!remote) {
        LOGE("Failed to get remote object");
        return -ENOMEM;
    }

    dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
    if (!dataShareHelper) {
        LOGE("Failed to connect to datashare");
        return -E_PERMISSION;
    }
    Uri uri(path);
    int fd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(flags));
    return fd;
}

static std::tuple<int, std::string> OpenByFileDataUri(Uri &uri, const std::string &uriStr, unsigned int mode)
{
    std::string bundleName = uri.GetAuthority();
    AppFileService::ModuleFileUri::FileUri fileUri(uriStr);
    std::string realPath = fileUri.GetRealPath();
    if ((bundleName == MEDIA || bundleName == DOCS) && access(realPath.c_str(), F_OK) != 0) {
        int res = OpenFileByDatashare(uri.ToString(), mode);
        if (res < 0) {
            LOGE("Failed to open file by Datashare error %{public}d", res);
        }
        return { res, uri.ToString() };
    }
    int ret = OpenFileByPath(realPath, mode);
    if (ret < 0) {
        if (bundleName == MEDIA) {
            ret = OpenFileByDatashare(uriStr, mode);
            if (ret < 0) {
                LOGE("Failed to open file by Datashare error %{public}d", ret);
            }
        } else {
            LOGE("Failed to open file for libuv error %{public}d", ret);
        }
    }
    return { ret, uriStr };
}

static std::tuple<int, std::string> OpenFileByBroker(const Uri &uri, uint32_t mode)
{
    uint32_t flag = (mode % MAX_WANT_FLAG) > 0 ?
        AAFwk::Want::FLAG_AUTH_WRITE_URI_PERMISSION :
        AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION;
    int ret = AAFwk::AbilityManagerClient::GetInstance()->OpenFile(uri, flag);
    if (ret < 0) {
        LOGE("Failed to open file by Broker error %{public}d", ret);
    }
    return { ret, uri.ToString() };
}

static std::tuple<int, std::string> CheckDataShareUri(const std::string &path, uint32_t mode)
{
    // datashare:////#fdFromBinder=xx
    int fd = -1;
    if (RemoteUri::IsRemoteUri(path, fd, mode)) {
        if (fd >= 0) {
            return { fd, path };
        }
        LOGE("Failed to open file by RemoteUri");
    }
    return { -EINVAL, path };
}

static std::tuple<int, std::string> OpenFileByUri(const std::string &path, uint32_t mode)
{
    Uri uri(path);
    std::string uriType = uri.GetScheme();
    if (uriType == SCHEME_FILE) {
        return OpenByFileDataUri(uri, path, mode);
    } else if (uriType == SCHEME_BROKER) {
        return OpenFileByBroker(uri, mode);
    } else if (uriType == DATASHARE) {
        return CheckDataShareUri(path, mode);
    }
    LOGE("Failed to open file by invalid uri");
    return { -EINVAL, path };
}

FileEntity* InstantiateFile(int fd, const std::string& pathOrUri, bool isUri)
{
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        LOGE("Failed to request heap memory.");
        close(fd);
        return nullptr;
    }
    FileEntity *fileEntity = new(std::nothrow) FileEntity();
    if (fileEntity == nullptr) {
        return nullptr;
    }
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return fileEntity;
}

static tuple<bool, unsigned int> GetCjFlags(int64_t mode)
{
    unsigned int flags = O_RDONLY;
    unsigned int invalidMode = (O_WRONLY | O_RDWR);
    if (mode < 0 || (static_cast<unsigned int>(mode) & invalidMode) == invalidMode) {
        LOGE("Invalid mode");
        return { false, flags };
    }
    flags = static_cast<unsigned int>(mode);
    (void)CommonFunc::ConvertCjFlags(flags);
    return { true, flags };
}
}

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {
using namespace OHOS::FFI;
std::tuple<int32_t, sptr<FileEntity>> FileEntity::Open(const char* path, int64_t mode)
{
    auto [succMode, flags] = GetCjFlags(mode);
    if (!succMode) {
        return {EINVAL, nullptr};
    }
    std::string pathStr(path);
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (pathStr.find("://") != string::npos) {
        auto [res, uriStr] = OpenFileByUri(pathStr, flags);
        if (res < 0) {
            return {res, nullptr};
        }
        auto fileEntity = InstantiateFile(res, uriStr, true);
        if (fileEntity == nullptr) {
            return { ENOMEM, nullptr};
        }
        auto fileUri = FFIData::Create<FileEntity>(std::move(fileEntity->fd_), fileEntity->path_, fileEntity->uri_);
        if (!fileUri) {
            delete(fileEntity);
            fileEntity = nullptr;
            return {ENOMEM, nullptr};
        }
        return {SUCCESS_CODE, fileUri};
    }
#endif
    int ret = OpenFileByPath(pathStr, flags);
    if (ret < 0) {
        LOGE("Failed to open file for libuv error %{public}d", ret);
        return {ret, nullptr};
    }
    auto file = InstantiateFile(ret, pathStr, false);
    if (file == nullptr) {
        return { ENOMEM, nullptr};
    }
    auto filePath = FFIData::Create<FileEntity>(std::move(file->fd_), file->path_, file->uri_);
    if (!filePath) {
        delete(file);
        file = nullptr;
        return {ENOMEM, nullptr};
    }
    return {SUCCESS_CODE, filePath};
}

std::tuple<int32_t, sptr<FileEntity>> FileEntity::Dup(int32_t fd)
{
    LOGI("FS_TEST::FileEntity::Dup start");
    if (fd < 0) {
        LOGE("Invalid fd");
        return {EINVAL, nullptr};
    }
    int dstFd = dup(fd);
    if (dstFd < 0) {
        LOGE("Failed to dup fd, errno: %{public}d", errno);
        return {errno, nullptr};
    }
    unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> readlink_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!readlink_req) {
        LOGE("Failed to request heap memory.");
        return {ENOMEM, nullptr};
    }
    string path = "/proc/self/fd/" + to_string(dstFd);
    int ret = uv_fs_readlink(nullptr, readlink_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to readlink fd, ret: %{public}d", ret);
        return {ret, nullptr};
    }
    auto fdPrt = CreateUniquePtr<DistributedFS::FDGuard>(dstFd, false);
    if (fdPrt == nullptr) {
        LOGE("Failed to request heap memory.");
        return {ENOMEM, nullptr};
    }
    auto pathStr = string(static_cast<const char *>(readlink_req->ptr));
    auto fileEntity = FFIData::Create<FileEntity>(std::move(fdPrt), pathStr, "");
    if (!fileEntity) {
        return {ENOMEM, nullptr};
    }
    return {SUCCESS_CODE, fileEntity};
}

static tuple<int, unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*>> RealPathCore(const string &srcPath)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> realpath_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!realpath_req) {
        LOGE("Failed to request heap memory.");
        return { ENOMEM, move(realpath_req)};
    }
    int ret = uv_fs_realpath(nullptr, realpath_req.get(), srcPath.c_str(), nullptr);
    if (ret < 0) {
        LOGE("Failed to realpath, ret: %{public}d", ret);
        return { ret, move(realpath_req)};
    }
    return { ERRNO_NOERR, move(realpath_req) };
}

int FileEntity::GetFD(int64_t id)
{
    auto fileEntity = FFIData::GetData<FileEntity>(id);
    if (!fileEntity) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    return fileEntity->fd_.get()->GetFD();
}

const char* FileEntity::GetPath(int64_t id)
{
    auto fileEntity = FFIData::GetData<FileEntity>(id);
    if (!fileEntity) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return nullptr;
    }
    auto [realPathRes, realPath] = RealPathCore(fileEntity->path_);
    if (realPathRes != ERRNO_NOERR) {
        return nullptr;
    }
    string tempPath = string(static_cast<char*>(realPath->ptr));
    char* value = static_cast<char*>(malloc((tempPath.size() + 1) * sizeof(char)));
    if (value == nullptr) {
        return nullptr;
    }
    if (strcpy_s(value, tempPath.size() + 1, tempPath.c_str()) != 0) {
        free(value);
        value = nullptr;
        return nullptr;
    }
    return value;
}

const char* FileEntity::GetName(int64_t id)
{
    string path = FileEntity::GetPath(id);
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        LOGE("Failed to split filename from path");
        return nullptr;
    }
    auto name = path.substr(pos + 1);
    char* value = static_cast<char*>(malloc((name.size() + 1) * sizeof(char)));
    if (value == nullptr) {
        return nullptr;
    }
    if (strcpy_s(value, name.size() + 1, name.c_str()) != 0) {
        free(value);
        return nullptr;
    }
    return value;
}

int FileEntity::TryLock(int64_t id, bool exclusive)
{
    auto fileEntity = FFIData::GetData<FileEntity>(id);
    if (!fileEntity) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    int ret = 0;
    auto mode = exclusive ? LOCK_EX : LOCK_SH;
    ret = flock(fileEntity->fd_.get()->GetFD(), static_cast<unsigned int>(mode) | LOCK_NB);
    if (ret < 0) {
        LOGE("Failed to try to lock file");
        return GetErrorCode(ETXTBSY);
    }
    return SUCCESS_CODE;
}

int FileEntity::UnLock(int64_t id)
{
    auto fileEntity = FFIData::GetData<FileEntity>(id);
    if (!fileEntity) {
        LOGE("FileEntity instance not exist %{public}" PRId64, id);
        return ERR_INVALID_INSTANCE_CODE;
    }
    int ret = 0;
    ret = flock(fileEntity->fd_.get()->GetFD(), LOCK_UN);
    if (ret < 0) {
        LOGE("Failed to unlock file");
        return GetErrorCode(ETXTBSY);
    }
    return SUCCESS_CODE;
}

RetDataCString FileEntity::GetParent()
{
    LOGI("FS_TEST::FileEntity::GetParent start");
    RetDataCString ret = { .code = EINVAL, .data = nullptr };
    string path(path_);
    if (uri_.length() != 0) {
        AppFileService::ModuleFileUri::FileUri fileUri(uri_);
        path = fileUri.GetRealPath();
    }
    auto [realPathRes, realPath] = RealPathCore(path);
    if (realPathRes != ERRNO_NOERR) {
        LOGE("realPath error");
        ret.code = realPathRes;
        return ret;
    }
    path = string(static_cast<const char *>(realPath->ptr));
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        LOGE("Failed to split filename from path");
        ret.code = ENOENT;
        return ret;
    }
    ret.code = SUCCESS_CODE;
    auto parent = path.substr(0, pos);
    char* result = new(std::nothrow) char[parent.length() + 1];
    if (result == nullptr) {
        ret.code = ENOMEM;
        return ret;
    }
    if (strcpy_s(result, parent.length() + 1, parent.c_str()) != 0) {
        ret.code = ENOMEM;
        delete[] (result);
        result = nullptr;
        return ret;
    }
    ret.data = result;
    LOGI("FS_TEST::FileEntity::GetParent success");
    return ret;
}

} // namespace FileFs
} // namespace CJSystemapi
} // namespace OHOS