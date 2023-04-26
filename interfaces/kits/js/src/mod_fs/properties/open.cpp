/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "open.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "ability.h"
#include "bundle_mgr_proxy.h"
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "common_func.h"
#include "datashare_helper.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "remote_uri.h"
#include "status_receiver_host.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::DistributedFS::ModuleRemoteUri;
using namespace OHOS::AppExecFwk;

static tuple<bool, unsigned int> GetJsFlags(napi_env env, const NFuncArg &funcArg)
{
    unsigned int mode = O_RDONLY;
    bool succ = false;
    if (funcArg.GetArgc() >= NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        unsigned int invalidMode = (O_WRONLY | O_RDWR);
        if (!succ || ((mode & invalidMode) == invalidMode)) {
            HILOGE("Invalid mode");
            NError(EINVAL).ThrowErr(env);
            return { false, mode };
        }
        (void)CommonFunc::ConvertJsFlags(mode);
    }
    return { true, mode };
}

static string DealWithUriWithName(string str)
{
    static uint32_t MEET_COUNT = 6;
    uint32_t count = 0;
    uint32_t index;
    for (index = 0; index < str.length(); index++) {
        if (str[index] == '/') {
            count++;
        }
        if (count == MEET_COUNT) {
            break;
        }
    }
    if (count == MEET_COUNT) {
        str = str.substr(0, index);
    }
    return str;
}

static NVal InstantiateFile(napi_env env, int fd, string pathOrUri, bool isUri)
{
    napi_value objFile = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objFile) {
        HILOGE("Failed to instantiate class");
        NError(EIO).ThrowErr(env);
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return NVal();
    }

    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        NError(EIO).ThrowErr(env);
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return NVal();
    }
    auto fdg = make_unique<DistributedFS::FDGuard>(fd, false);
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return { env, objFile };
}

static int OpenFileByDatashare(string path, unsigned int flags)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    int fd = -1;
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
    path = DealWithUriWithName(path);
    Uri uri(path);
    fd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(flags));
    return fd;
}

static sptr<BundleMgrProxy> GetBundleMgrProxy()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("Failed to get system ability mgr.");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("Failed to get bundle manager proxy.");
        return nullptr;
    }
    return iface_cast<BundleMgrProxy>(remoteObject);
}

static string GetBundleNameSelf()
{
    int uid = -1;
    uid = IPCSkeleton::GetCallingUid();
    
    sptr<BundleMgrProxy> bundleMgrProxy = GetBundleMgrProxy();
    if (!bundleMgrProxy) {
        HILOGE("Bundle mgr proxy is null ptr.");
        return nullptr;
    }
    string bundleName;
    if (!bundleMgrProxy->GetBundleNameForUid(uid, bundleName)) {
        HILOGE("Failed to get bundleNameSelf. uid is %{public}d", uid);
        return nullptr;
    }
    return bundleName;
}

static string GetPathFromFileUri(string path, string bundleName, unsigned int mode)
{
    if (bundleName != GetBundleNameSelf()) {
        if ((mode & O_WRONLY) == O_WRONLY || (mode & O_RDWR) == O_RDWR) {
            path = PATH_SHARE + MODE_RW + bundleName + path;
        } else {
            path = PATH_SHARE + MODE_R + bundleName + path;
        }
    }
    return path;
}

napi_value Open::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        HILOGE("Invalid mode");
        return nullptr;
    }
    int fd = -1;
    string pathStr = string(path.get());
    if (RemoteUri::IsMediaUri(pathStr)) {
        int ret = OpenFileByDatashare(pathStr, mode);
        if (ret >= 0) {
            auto file = InstantiateFile(env, ret, pathStr, true).val_;
            return file;
        }
        HILOGE("Failed to open file by Datashare");
        NError(-ret).ThrowErr(env);
        return nullptr;
    } else if (RemoteUri::IsFileUri(pathStr)) {
        RemoteUri remoteUri = RemoteUri(pathStr);
        pathStr = GetPathFromFileUri(remoteUri.GetPath(), remoteUri.GetAuthority(), mode);
    } else if (RemoteUri::IsRemoteUri(pathStr, fd, mode)) {
        if (fd >= 0) {
            auto file = InstantiateFile(env, fd, pathStr, true).val_;
            return file;
        }
        HILOGE("Failed to open file by RemoteUri");
        NError(E_INVAL).ThrowErr(env);
        return nullptr;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), pathStr.c_str(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    auto file = InstantiateFile(env, ret, pathStr, false).val_;
    return file;
}

struct AsyncOpenFileArg {
    int fd;
    string path;
    string uri;
};

napi_value Open::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        HILOGE("Invalid mode");
        return nullptr;
    }
    auto arg = make_shared<AsyncOpenFileArg>();
    auto argv = funcArg[NARG_POS::FIRST];
    auto cbExec = [arg, argv, path = string(path.get()), mode = mode, env = env]() -> NError {
        string pathStr = path;
        int fd = -1;
        if (RemoteUri::IsMediaUri(path)) {
            int ret = OpenFileByDatashare(path, mode);
            if (ret >= 0) {
                arg->fd = ret;
                arg->path = "";
                arg->uri = path;
                return NError(ERRNO_NOERR);
            }
            HILOGE("Failed to open file by Datashare");
            return NError(-ret);
        } else if (RemoteUri::IsFileUri(path)) {
            RemoteUri remoteUri = RemoteUri(path);
            pathStr = GetPathFromFileUri(remoteUri.GetPath(), remoteUri.GetAuthority(), mode);
        } else if (RemoteUri::IsRemoteUri(path, fd, mode)) {
            if (fd >= 0) {
                arg->fd = fd;
                arg->path = "";
                arg->uri = path;
                return NError(ERRNO_NOERR);
            }
            HILOGE("Failed to open file by RemoteUri");
            return NError(E_INVAL);
        }
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), pathStr.c_str(), mode, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
            return NError(errno);
        }
        arg->fd = ret;
        arg->path = pathStr;
        arg->uri = "";
        return NError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        bool isUri = false;
        if (arg->path.empty() && arg->uri.size()) {
            isUri = true;
            return InstantiateFile(env, arg->fd, arg->uri, isUri);
        }
        return InstantiateFile(env, arg->fd, arg->path, isUri);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_OPEN_NAME, cbExec, cbCompl).val_;
    } else {
        int cbIdx = ((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::SECOND);
        NVal cb(env, funcArg[cbIdx]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_OPEN_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS