/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "open_core.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "file_entity.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "ability_manager_client.h"
#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "datashare_helper.h"
#include "file_uri.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "remote_uri.h"
#include "status_receiver_host.h"
#include "system_ability_definition.h"
#endif

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
using namespace OHOS::DistributedFS::ModuleRemoteUri;
using namespace OHOS::AppExecFwk;
#endif

const std::string PROCEDURE_OPEN_NAME = "FileIOOpen";
const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
const std::string FILE_DATA_URI = "file://";
const std::string PATH_SHARE = "/data/storage/el2/share";
const std::string MODE_RW = "/rw/";
const std::string MODE_R = "/r/";
const std::string MEDIA = "media";
const std::string DOCS = "docs";
const std::string DATASHARE = "datashare";
const std::string SCHEME_BROKER = "content";
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
constexpr uint32_t MAX_WANT_FLAG = 4;
#endif

static tuple<bool, uint32_t> ValidAndConvertFlags(const optional<int32_t> &mode)
{
    uint32_t flags = O_RDONLY;
    if (mode.has_value()) {
        auto modeValue = mode.value();
        int32_t invalidMode = (O_WRONLY | O_RDWR);
        if (modeValue < 0 || ((modeValue & invalidMode) == invalidMode)) {
            HILOGE("Invalid mode");
            return { false, flags };
        }
        flags = static_cast<uint32_t>(modeValue);
        (void)FsUtils::ConvertFlags(flags);
    }
    return { true, flags };
}

static FsResult<FsFile *> InstantiateFile(int fd, string pathOrUri, bool isUri)
{
    FsResult<FsFile *> result = FsFile::Constructor();
    if (!result.IsSuccess()) {
        HILOGE("Failed to instantiate class");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return FsResult<FsFile *>::Error(EIO);
    }

    const FsFile *objFile = result.GetData().value();
    if (!objFile) {
        HILOGE("Failed to get fsFile");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return FsResult<FsFile *>::Error(EIO);
    }

    auto *fileEntity = objFile->GetFileEntity();
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        delete objFile;
        objFile = nullptr;
        return FsResult<FsFile *>::Error(EIO);
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(fd);
        delete objFile;
        objFile = nullptr;
        return FsResult<FsFile *>::Error(ENOMEM);
    }
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return result;
}

static int OpenFileByPath(const string &path, uint32_t mode)
{
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> open_req = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    return ret;
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)

static int OpenFileByDatashare(const string &path, uint32_t flags)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    sptr<FileIoToken> remote = new (std::nothrow) IRemoteStub<FileIoToken>();
    if (!remote) {
        HILOGE("Failed to get remote object");
        return -ENOMEM;
    }

    dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
    if (!dataShareHelper) {
        HILOGE("Failed to connect to datashare");
        return -E_PERMISSION;
    }
    Uri uri(path);
    int fd = dataShareHelper->OpenFile(uri, FsUtils::GetModeFromFlags(flags));
    return fd;
}

static tuple<int, string> OpenByFileDataUri(Uri &uri, const string &uriStr, uint32_t mode)
{
    string bundleName = uri.GetAuthority();
    AppFileService::ModuleFileUri::FileUri fileUri(uriStr);
    string realPath = fileUri.GetRealPath();
    if (bundleName == MEDIA) {
        int res = OpenFileByDatashare(uri.ToString(), mode);
        if (res < 0) {
            HILOGE("Failed to open file by Datashare error %{public}d", res);
        }
        return { res, uri.ToString() };
    } else if (bundleName == DOCS && access(realPath.c_str(), F_OK) != 0) {
        int res = OpenFileByDatashare(uri.ToString(), mode);
        if (res < 0) {
            HILOGE("Failed to open file by Datashare error %{public}d", res);
            return { -ENOENT, uri.ToString() };
        }
        return { res, uri.ToString() };
    }
    int ret = OpenFileByPath(realPath, mode);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
    }
    return { ret, uriStr };
}

static tuple<int, string> OpenFileByBroker(const Uri &uri, uint32_t mode)
{
    uint32_t flag = (mode % MAX_WANT_FLAG) > 0 ? AAFwk::Want::FLAG_AUTH_WRITE_URI_PERMISSION
                                            : AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION;
    int ret = AAFwk::AbilityManagerClient::GetInstance()->OpenFile(uri, flag);
    if (ret < 0) {
        HILOGE("Failed to open file by Broker error %{public}d", ret);
    }
    return { ret, uri.ToString() };
}

static tuple<int, string> OpenFileByUri(const string &path, uint32_t mode)
{
    Uri uri(path);
    string uriType = uri.GetScheme();
    if (uriType == SCHEME_FILE) {
        return OpenByFileDataUri(uri, path, mode);
    }
    if (uriType == SCHEME_BROKER) {
        return OpenFileByBroker(uri, mode);
    }
    int fd = -1;
    if (uriType == DATASHARE && RemoteUri::IsRemoteUri(path, fd, mode)) {
        if (fd >= 0) {
            return { fd, path };
        }
        HILOGE("Failed to open file by RemoteUri");
    }
    HILOGE("Failed to open file by invalid uri");
    return { -EINVAL, path };
}
#endif

FsResult<FsFile *> OpenCore::DoOpen(const string &path, const optional<int32_t> &mode)
{
#ifdef FILE_API_TRACE
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
#endif
    auto [succMode, modeValue] = ValidAndConvertFlags(mode);
    if (!succMode) {
        return FsResult<FsFile *>::Error(EINVAL);
    }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (path.find("://") != string::npos) {
        auto [res, uriStr] = OpenFileByUri(path, modeValue);
        if (res < 0) {
            return FsResult<FsFile *>::Error(res);
        }
        return InstantiateFile(res, uriStr, true);
    }
#endif
    int ret = OpenFileByPath(path, modeValue);
    if (ret < 0) {
        HILOGD("Failed to open file for libuv error %{public}d", ret);
        return FsResult<FsFile *>::Error(ret);
    }
    return InstantiateFile(ret, path, false);
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS