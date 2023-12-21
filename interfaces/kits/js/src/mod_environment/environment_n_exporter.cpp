/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "environment_n_exporter.h"
#include "accesstoken_kit.h"
#include "account_error_no.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "parameter.h"
#include "tokenid_kit.h"
#include <string>
#include <unistd.h>
namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
using namespace OHOS::FileManagement::LibN;
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
const char *g_fileManagerFullMountEnableParameter = "const.filemanager.full_mount.enable";
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
    int retSystem = GetParameter(g_fileManagerFullMountEnableParameter, "false", value, sizeof(value));
    if (retSystem > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    HILOGE("Not supporting all mounts");
    return false;
}

static bool CheckInvalidAccess(const std::string &permission, napi_env env)
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGE("Capability not supported");
        NError(E_DEVICENOTSUPPORT).ThrowErr(env);
        return true;
    }
    if (permission == FILE_ACCESS_MANAGER_PERMISSION) {
        if (!IsSystemApp()) {
            NError(E_PERMISSION_SYS).ThrowErr(env);
            return true;
        }
    }
    if (!CheckCallingPermission(permission)) {
        HILOGE("No Permission");
        NError(E_PERMISSION).ThrowErr(env);
        return true;
    }
    return false;
}
}

napi_value GetStorageDataDir(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, STORAGE_DATA_PATH);
    };

    static const std::string PROCEDURE_NAME = "GetStorageDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

int GetUserId()
{
    return 0;
}

napi_value GetUserDataDir(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto userDataPath = std::make_shared<std::string>();
    auto cbExec = [userDataPath]() -> NError {
        (*userDataPath).append("/storage/media/").append(std::to_string(GetUserId())).append("/local");
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [userDataPath](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUTF8String(env, *userDataPath);
    };

    static const std::string PROCEDURE_NAME = "GetUserDataDir";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetUserDownloadDir(napi_env env, napi_callback_info info)
{
    if (CheckInvalidAccess(READ_WRITE_DOWNLOAD_PERMISSION, env)) {
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    static std::string downloadPath = GetPublicPath(DOWNLOAD_PATH);
    if (downloadPath.empty()) {
        HILOGE("Unknown error");
        NError(E_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, downloadPath).val_;
}

napi_value GetUserDesktopDir(napi_env env, napi_callback_info info)
{
    if (CheckInvalidAccess(READ_WRITE_DESKTOP_PERMISSION, env)) {
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    static std::string desktopPath = GetPublicPath(DESKTOP_PATH);
    if (desktopPath.empty()) {
        HILOGE("Unknown error");
        NError(E_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, desktopPath).val_;
}

napi_value GetUserDocumentDir(napi_env env, napi_callback_info info)
{
    if (CheckInvalidAccess(READ_WRITE_DOCUMENTS_PERMISSION, env)) {
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    static std::string documentsPath = GetPublicPath(DOCUMENTS_PATH);
    if (documentsPath.empty()) {
        HILOGE("Unknown error");
        NError(E_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, documentsPath).val_;
}

napi_value GetExternalStorageDir(napi_env env, napi_callback_info info)
{
    if (CheckInvalidAccess(FILE_ACCESS_MANAGER_PERMISSION, env)) {
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, EXTERNAL_STORAGE_PATH).val_;
}

napi_value GetUserHomeDir(napi_env env, napi_callback_info info)
{
    if (CheckInvalidAccess(FILE_ACCESS_MANAGER_PERMISSION, env)) {
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    static std::string userName = GetUserName();
    if (userName.empty()) {
        HILOGE("Unknown error");
        NError(E_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, PC_STORAGE_PATH + userName).val_;
}
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS