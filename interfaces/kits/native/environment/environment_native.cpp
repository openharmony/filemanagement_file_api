/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "environment_native.h"

#include <string>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "parameter.h"
#include "securec.h"
#include "tokenid_kit.h"

using namespace OHOS;
using namespace OHOS::FileManagement;
namespace {
const std::string STORAGE_PATH = "/storage/Users/";
const std::string READ_WRITE_DOWNLOAD_PERMISSION = "ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY";
const std::string READ_WRITE_DESKTOP_PERMISSION = "ohos.permission.READ_WRITE_DESKTOP_DIRECTORY";
const std::string READ_WRITE_DOCUMENTS_PERMISSION = "ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY";
const std::string DOWNLOAD_PATH = "/Download";
const std::string DESKTOP_PATH = "/Desktop";
const std::string DOCUMENTS_PATH = "/Documents";
const std::string DEFAULT_USERNAME = "currentUser";
const std::string FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER = "const.filemanager.full_mount.enable";
const int PERMISSION_ERROR = 201;
const int DEVICE_NOT_SUPPORTED = 801;

static bool CheckCallingPermission(const std::string &permission)
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    return Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permission) !=
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

static std::string GetUserName()
{
    return DEFAULT_USERNAME;
}

static std::string GetPublicPath(const std::string &directoryName)
{
    return STORAGE_PATH + GetUserName() + directoryName;
}

static bool CheckFileManagerFullMountEnable()
{
    char value[] = "false";
    int retSystem = GetParameter(FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER.c_str(), "false", value, sizeof(value));
    return (retSystem > 0) && (!std::strcmp(value, "true"));
}

static int CheckInvalidAccess(const std::string &permission)
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGD("Failed to enable the parameter: %{public}s", FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER.c_str());
        return -DEVICE_NOT_SUPPORTED;
    }
    if (!CheckCallingPermission(permission)) {
        HILOGD("Failed to own the permission: %{public}s", permission.c_str());
        return -PERMISSION_ERROR;
    }
    return 0;
}
}

int GetUserDir(char *permission, char **result)
{
    std::string perm(permission);
    int ret = CheckInvalidAccess(perm);
    if (ret != 0) {
        return ret;
    }
    std::string path = "";
    if (perm == READ_WRITE_DESKTOP_PERMISSION) {
        path = GetPublicPath(DESKTOP_PATH);
    } else if (perm == READ_WRITE_DOCUMENTS_PERMISSION) {
        path = GetPublicPath(DOCUMENTS_PATH);
    } else if (perm == READ_WRITE_DOWNLOAD_PERMISSION) {
        path = GetPublicPath(DOWNLOAD_PATH);
    }
    *result = (char *) malloc((path.length() + 1) * sizeof(char));
    if (*result == nullptr) {
        return -ENOMEM;
    }
    ret = strcpy_s(*result, (path.length() + 1) * sizeof(char), path.c_str());
    if (ret != 0) {
        HILOGE("Failed to copy memory");
        return -ENOMEM;
    }
    return ret;
}
