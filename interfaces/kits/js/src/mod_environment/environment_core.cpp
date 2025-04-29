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

#include "environment_core.h"

#include <string>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "account_error_no.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
namespace {
const std::string STORAGE_DATA_PATH = "/data";
const std::string PC_STORAGE_PATH = "/storage/Users/";
const std::string EXTERNAL_STORAGE_PATH = "/storage/External";
const std::string USER_APP_DATA_PATH = "/appdata";
const std::string READ_WRITE_DOWNLOAD_PERMISSION = "ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY";
const std::string READ_WRITE_DESKTOP_PERMISSION = "ohos.permission.READ_WRITE_DESKTOP_DIRECTORY";
const std::string READ_WRITE_DOCUMENTS_PERMISSION = "ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY";
const std::string FILE_ACCESS_MANAGER_PERMISSION = "ohos.permission.FILE_ACCESS_MANAGER";
const std::string DOWNLOAD_PATH = "/Download";
const std::string DESKTOP_PATH = "/Desktop";
const std::string DOCUMENTS_PATH = "/Documents";
const std::string DEFAULT_USERNAME = "currentUser";
const char *FILE_MANAMER_FULL_MOUNT_ENABLE_PARAMETER = "const.filemanager.full_mount.enable";
static bool IsSystemApp()
{
    uint64_t fullTokenId = OHOS::IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
}

static bool CheckCallingPermission(const std::string &permission)
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permission);
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOGE("ModuleUserDirectory::CheckCallingPermission have no fileAccess permission");
        return false;
    }
    return true;
}

static std::string GetUserName()
{
    std::string userName;
    ErrCode errCode = OHOS::AccountSA::OsAccountManager::GetOsAccountShortName(userName);
    if (errCode != ERR_OK || userName.empty()) {
        HILOGE("Get userName Failed");
    }
    userName = DEFAULT_USERNAME;
    return userName;
}

static std::string GetPublicPath(const std::string &directoryName)
{
    return PC_STORAGE_PATH + GetUserName() + directoryName;
}

static bool CheckFileManagerFullMountEnable()
{
    char value[] = "false";
    int retSystem = GetParameter(FILE_MANAMER_FULL_MOUNT_ENABLE_PARAMETER, "false", value, sizeof(value));
    if (retSystem > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    HILOGE("Not supporting all mounts");
    return false;
}

static int CheckInvalidAccess(const std::string &permission)
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGE("Capability not supported");
        return E_DEVICENOTSUPPORT;
    }
    if (permission == FILE_ACCESS_MANAGER_PERMISSION) {
        if (!IsSystemApp()) {
            return E_PERMISSION_SYS;
        }
    }
    if (!CheckCallingPermission(permission)) {
        HILOGE("No Permission");
        return E_PERMISSION;
    }
    return ERRNO_NOERR;
}
}

FsResult<std::string> DoGetStorageDataDir()
{
    if (!IsSystemApp()) {
        return FsResult<std::string>::Error(E_PERMISSION_SYS);
    }
    return FsResult<std::string>::Success(std::move(STORAGE_DATA_PATH));
}

int GetUserId()
{
    return 0;
}

FsResult<std::string> DoGetUserDataDir()
{
    if (!IsSystemApp()) {
        return FsResult<std::string>::Error(E_PERMISSION_SYS);
    }

    auto userDataPath = std::make_shared<std::string>();
    (*userDataPath).append("/storage/media/").append(std::to_string(GetUserId())).append("/local");
    return FsResult<std::string>::Success(std::move(*userDataPath));
}

FsResult<std::string> DoGetUserDownloadDir()
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGE("Capability not supported");
        return FsResult<std::string>::Error(E_DEVICENOTSUPPORT);
    }

    static std::string downloadPath = GetPublicPath(DOWNLOAD_PATH);
    if (downloadPath.empty()) {
        HILOGE("Unknown error");
        return FsResult<std::string>::Error(E_UNKNOWN_ERROR);
    }
    return FsResult<std::string>::Success(std::move(downloadPath));
}

FsResult<std::string> DoGetUserDesktopDir()
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGE("Capability not supported");
        return FsResult<std::string>::Error(E_DEVICENOTSUPPORT);
    }

    static std::string desktopPath = GetPublicPath(DESKTOP_PATH);
    if (desktopPath.empty()) {
        HILOGE("Unknown error");
        return FsResult<std::string>::Error(E_UNKNOWN_ERROR);
    }
    return FsResult<std::string>::Success(std::move(desktopPath));
}

FsResult<std::string> DoGetUserDocumentDir()
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGE("Capability not supported");
        return FsResult<std::string>::Error(E_DEVICENOTSUPPORT);
    }

    static std::string documentsPath = GetPublicPath(DOCUMENTS_PATH);
    if (documentsPath.empty()) {
        HILOGE("Unknown error");
        return FsResult<std::string>::Error(E_UNKNOWN_ERROR);
    }
    return FsResult<std::string>::Success(std::move(documentsPath));
}

FsResult<std::string> DoGetExternalStorageDir()
{
    auto res = CheckInvalidAccess(FILE_ACCESS_MANAGER_PERMISSION);
    if (res) {
        return FsResult<std::string>::Error(res);
    }
    return FsResult<std::string>::Success(std::move(EXTERNAL_STORAGE_PATH));
}

FsResult<std::string> DoGetUserHomeDir()
{
    auto res = CheckInvalidAccess(FILE_ACCESS_MANAGER_PERMISSION);
    if (res) {
        return FsResult<std::string>::Error(res);
    }

    static std::string userName = GetUserName();
    if (userName.empty()) {
        HILOGE("Unknown error");
        return FsResult<std::string>::Error(E_UNKNOWN_ERROR);
    }
    std::string result = PC_STORAGE_PATH + userName;
    return FsResult<std::string>::Success(std::move(result));
}
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS