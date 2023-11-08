/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "userdirectory_n_exporter.h"
#include <string>

#include "accesstoken_kit.h"
#include "account_error_no.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "tokenid_kit.h"
namespace OHOS {
namespace FileManagement {
namespace ModuleUserDirectory {
using namespace OHOS::FileManagement::LibN;
namespace {
const std::string PC_STORAGE_PATH = "/storage/Users/";
const std::string EXTERNAL_STORAGE_PATH = "/storage/External";
const std::string USER_APP_DATA_PATH = "/appdata";
const std::string READ_WRITE_DOWNLOAD_PERMISSION = "ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY";
const std::string READ_WRITE_DESKTOP_PERMISSION = "ohos.permission.READ_WRITE_DESKTOP_DIRECTORY";
const std::string READ_WRITE_DOCUMENTS_PERMISSION = "ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY";
const std::string DOWNLOAD_PATH = "/Download";
const std::string DESKTOP_PATH = "/Desktop";
const std::string DOCUMENTS_PATH = "/Documents";

bool IsSystemApp()
{
    uint64_t fullTokenId = OHOS::IPCSkeleton::GetCallingFullTokenID();
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
}

bool CheckCallingPermission(const std::string &permission)
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
    if (errCode != ERR_OK) {
        HILOGE("Get userName Failed");
        return nullptr;
    }
    return userName;
}

static std::string GetPublicPath(const std::string &directoryName)
{
    std::string userName = GetUserName();
    if (userName.empty()) {
        HILOGE("Get PublicPath Failed");
        return nullptr;
    }
    return PC_STORAGE_PATH + userName + directoryName;
}
}

napi_value GetUserDownloadDir(napi_env env, napi_callback_info info)
{
    if (!CheckCallingPermission(READ_WRITE_DOWNLOAD_PERMISSION)) {
        HILOGE("No Permission");
        NError(E_PERMISSION).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, GetPublicPath(DOWNLOAD_PATH)).val_;
}

napi_value GetUserDesktopDir(napi_env env, napi_callback_info info)
{
    if (!CheckCallingPermission(READ_WRITE_DESKTOP_PERMISSION)) {
        HILOGE("No Permission");
        NError(E_PERMISSION).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, GetPublicPath(DESKTOP_PATH)).val_;
}

napi_value GetUserDocumentsDir(napi_env env, napi_callback_info info)
{
    if (!CheckCallingPermission(READ_WRITE_DOCUMENTS_PERMISSION)) {
        HILOGE("No Permission");
        NError(E_PERMISSION).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, GetPublicPath(DOCUMENTS_PATH)).val_;
}

napi_value GetExternalStorageDir(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
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
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, PC_STORAGE_PATH + GetUserName()).val_;
}
} // namespace ModuleUserDirectory
} // namespace FileManagement
} // namespace OHOS