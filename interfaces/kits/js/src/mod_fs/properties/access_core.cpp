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

#include "access_core.h"

#include <unistd.h>

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <sys/xattr.h>

#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "rust_file.h"
#include "system_ability_definition.h"

#endif

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

#include "file_fs_trace.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace AppExecFwk;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
const string CLOUDDISK_FILE_PREFIX = "/data/storage/el2/cloud";
const string DISTRIBUTED_FILE_PREFIX = "/data/storage/el2/distributedfiles";
const string PACKAGE_NAME_FLAG = "<PackageName>";
const string USER_ID_FLAG = "<currentUserId>";
const string PHYSICAL_PATH_PREFIX = "/mnt/hmdfs/<currentUserId>/account/device_view/local/data/<PackageName>";
const string CLOUD_FILE_LOCATION = "user.cloud.location";
const char POSITION_LOCAL = '1';
const char POSITION_BOTH = '3';
const int BASE_USER_RANGE = 200000;
#endif

static int UvAccess(const string &path, int mode)
{
    FileFsTrace traceUvAccess("UvAccess");
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> accessReq = {new uv_fs_t, FsUtils::FsReqCleanup};
    if (!accessReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_access(nullptr, accessReq.get(), path.c_str(), mode, nullptr);
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
static bool IsCloudOrDistributedFilePath(const string &path)
{
    return path.find(CLOUDDISK_FILE_PREFIX) == 0 || path.find(DISTRIBUTED_FILE_PREFIX) == 0;
}

static int GetCurrentUserId()
{
    int uid = IPCSkeleton::GetCallingUid();
    int userId = uid / BASE_USER_RANGE;
    return userId;
}

static sptr<BundleMgrProxy> GetBundleMgrProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy");
        return nullptr;
    }

    return iface_cast<BundleMgrProxy>(remoteObject);
}

static string GetSelfBundleName()
{
    sptr<BundleMgrProxy> bundleMgrProxy = GetBundleMgrProxy();
    if (!bundleMgrProxy) {
        HILOGE("bundleMgrProxy is nullptr");
        return "";
    }
    BundleInfo bundleInfo;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(0, bundleInfo);
    if (ret != 0) {
        HILOGE("bundleName get fail");
        return "";
    }
    return bundleInfo.name;
}

static int HandleLocalCheck(const string &path, int mode)
{
    // check if the file of /data/storage/el2/cloud is on the local
    if (path.find(CLOUDDISK_FILE_PREFIX) == 0) {
        char val[2] = {'\0'};
        if (getxattr(path.c_str(), CLOUD_FILE_LOCATION.c_str(), val, sizeof(val)) < 0) {
            HILOGI("get cloud file location fail, err: %{public}d", errno);
            return errno;
        }
        if (val[0] == POSITION_LOCAL || val[0] == POSITION_BOTH) {
            return 0;
        }
        return ENOENT;
    }
    // check if the distributed file of /data/storage/el2/distributedfiles is on the local,
    // convert into physical path(/mnt/hmdfs/<currentUserId>/account/device_view/local/data/<PackageName>) and check
    if (path.find(DISTRIBUTED_FILE_PREFIX) == 0) {
        int userId = GetCurrentUserId();
        string bundleName = GetSelfBundleName();
        string relativePath = path.substr(DISTRIBUTED_FILE_PREFIX.length());
        string physicalPath = PHYSICAL_PATH_PREFIX + relativePath;
        physicalPath.replace(physicalPath.find(USER_ID_FLAG), USER_ID_FLAG.length(), to_string(userId));
        physicalPath.replace(physicalPath.find(PACKAGE_NAME_FLAG), PACKAGE_NAME_FLAG.length(), bundleName);

        return UvAccess(physicalPath, mode);
    }

    return ENOENT;
}
#endif

static int Access(const string &path, int mode, int flag)
{
    if (FileApiDebug::isLogEnabled) {
        HILOGD("Path is %{public}s, mode is %{public}d", path.c_str(), mode);
    }
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (flag == LOCAL_FLAG && IsCloudOrDistributedFilePath(path)) {
        return HandleLocalCheck(path, mode);
    }
#endif
    return UvAccess(path, mode);
}

static int GetMode(const std::optional<AccessModeType> &modeOpt, bool *hasMode)
{
    if (modeOpt.has_value()) {
        int mode = static_cast<int>(modeOpt.value());
        *hasMode = true;
        if ((static_cast<unsigned int>(mode) & 0x06) == static_cast<unsigned int>(mode)) {
            return mode;
        }
    }

    return -1;
}

static bool ValidAccessArgs(const std::string &path, const std::optional<AccessModeType> &modeOpt,
                            int &finalMode)
{
    if (path.empty()) {
        HILOGE("Invalid path");
        return false;
    }
    bool hasMode = false;
    int mode = GetMode(modeOpt, &hasMode);
    if (mode < 0 && hasMode) {
        HILOGE("Invalid mode");
        return false;
    }
    finalMode = hasMode ? mode : 0;
    return true;
}

FsResult<bool> AccessCore::DoAccess(const std::string& path, const std::optional<AccessModeType> &mode)
{
    FileFsTrace traceDoAccess("DoAccess");
    int finalMode = 0;
    int flag = DEFAULT_FLAG;
    if (!ValidAccessArgs(path, mode, finalMode)) {
        return FsResult<bool>::Error(EINVAL);
    }
    int ret = Access(path, finalMode, flag);
    if (ret < 0 && (std::string_view(uv_err_name(ret)) != "ENOENT")) {
        HILOGE("Failed to access file by path, ret:%{public}d", ret);
        return FsResult<bool>::Error(ret);
    }
    bool isAccess = (ret == 0);
    return FsResult<bool>::Success(isAccess);
}

FsResult<bool> AccessCore::DoAccess(const std::string& path, const AccessModeType &mode, const AccessFlag &flag)
{
    FileFsTrace traceDoAccess("DoAccess");
    int finalMode = static_cast<int>(mode);
    int finalFlag = static_cast<int>(flag);
    if (!ValidAccessArgs(path, std::make_optional(mode), finalMode)) {
        return FsResult<bool>::Error(EINVAL);
    }
    int ret = Access(path, finalMode, finalFlag);
    if (ret < 0 && (std::string_view(uv_err_name(ret)) != "ENOENT")) {
        HILOGE("Failed to access file by path");
        return FsResult<bool>::Error(ret);
    }
    bool isAccess = (ret == 0);
    return FsResult<bool>::Success(isAccess);
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS