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

#include "mkdir_core.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <sys/xattr.h>

#include "bundle_mgr_proxy.h"
#include "filemgmt_libhilog.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "rust_file.h"
#include "system_ability_definition.h"
#endif

#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

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

enum AccessFlag : int32_t {
    DEFAULT_FLAG = -1,
    LOCAL_FLAG,
};

struct AccessArgs {
    string path;
    int mode = -1;
    int flag = DEFAULT_FLAG;
};

static int UvAccess(const string &path, int mode)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> access_req = {new uv_fs_t,
        FsUtils::FsReqCleanup};
    if (!access_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_access(nullptr, access_req.get(), path.c_str(), mode, nullptr);
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

static int AccessCore(const string &path, int mode, int flag = DEFAULT_FLAG)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (flag == LOCAL_FLAG && IsCloudOrDistributedFilePath(path)) {
        return HandleLocalCheck(path, mode);
    }
#endif
    return UvAccess(path, mode);
}

static int MkdirCore(const string &path)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> mkdir_req = {
        new uv_fs_t, FsUtils::FsReqCleanup };
    if (!mkdir_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    return uv_fs_mkdir(nullptr, mkdir_req.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
}

static int32_t MkdirExec(const string &path, bool recursion, bool hasOption)
{
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (hasOption) {
        int ret = AccessCore(path, 0);
        if (ret == ERRNO_NOERR) {
            HILOGD("The path already exists");
            return EEXIST;
        }
        if (ret != -ENOENT) {
            HILOGE("Failed to check for illegal path or request for heap memory");
            return ret;
        }
        if (::Mkdirs(path.c_str(), static_cast<MakeDirectionMode>(recursion)) < 0) {
            HILOGD("Failed to create directories, error: %{public}d", errno);
            return errno;
        }
        ret = AccessCore(path, 0);
        if (ret) {
            HILOGE("Failed to verify the result of Mkdirs function");
            return ret;
        }
        return ERRNO_NOERR;
    }
#endif
    int ret = MkdirCore(path);
    if (ret) {
        HILOGD("Failed to create directory");
        return ret;
    }
    return ERRNO_NOERR;
}

FsResult<void> MkdirCore::DoMkdir(const std::string &path, const std::optional<bool> &recursion)
{
    bool hasOption = false;
    bool mkdirRecursion = false;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    hasOption = recursion.has_value();
    if (hasOption) {
        mkdirRecursion = recursion.value();
    }
#endif
    auto err = MkdirExec(path, mkdirRecursion, hasOption);
    if (err) {
        return FsResult<void>::Error(err);
    }
    return FsResult<void>::Success();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS