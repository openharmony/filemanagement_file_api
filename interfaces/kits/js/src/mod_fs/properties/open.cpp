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

#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "file_utils.h"
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "ability.h"
#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "datashare_helper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "remote_uri.h"
#include "status_receiver_host.h"
#include "system_ability_definition.h"
#include "file_uri.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
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

static tuple<bool, unsigned int> GetJsFlags(napi_env env, const NFuncArg &funcArg)
{
    unsigned int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        auto [succ, mode] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32(O_RDONLY);
        int32_t invalidMode = (O_WRONLY | O_RDWR);
        if (!succ || mode < 0 || ((mode & invalidMode) == invalidMode)) {
            HILOGE("Invalid mode");
            NError(EINVAL).ThrowErr(env);
            return { false, flags };
        }
        flags = static_cast<unsigned int>(mode);
        (void)CommonFunc::ConvertJsFlags(flags);
    }
    return { true, flags };
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
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return NVal();
    }
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

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)

static int OpenFileByDatashare(const string &path, unsigned int flags)
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
    Uri uri(path);
    fd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(flags));
    return fd;
}
#endif

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
        return nullptr;
    }
    string pathStr = string(path.get());
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    Uri uri(pathStr);
    string bundleName = uri.GetAuthority();
    string uriType = uri.GetScheme();
    string uriPath = uri.GetPath();
    if (uriType == SCHEME_FILE) {
        AppFileService::ModuleFileUri::FileUri fileUri(pathStr);
        pathStr = fileUri.GetRealPath();
        if ((bundleName == MEDIA || bundleName == DOCS) &&
            access(pathStr.c_str(), F_OK) != 0) {
            int ret = OpenFileByDatashare(uri.ToString(), mode);
            if (ret >= 0) {
                auto file = InstantiateFile(env, ret, uri.ToString(), true).val_;
                return file;
            }
            HILOGE("Failed to open file by Datashare error %{public}d", ret);
            NError(-ret).ThrowErr(env);
            return nullptr;
        }
    } else if (uriType == DATASHARE) {
        // datashare:////#fdFromBinder=xx
        int fd = -1;
        if (RemoteUri::IsRemoteUri(pathStr, fd, mode)) {
            if (fd >= 0) {
                auto file = InstantiateFile(env, fd, pathStr, true).val_;
                return file;
            }
            HILOGE("Failed to open file by RemoteUri");
            NError(E_INVAL).ThrowErr(env);
            return nullptr;
        }
    }
#endif
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
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        if (bundleName == MEDIA) {
            ret = OpenFileByDatashare(uri.ToString(), mode);
            if (ret < 0) {
                HILOGE("Failed to open file by Datashare error %{public}d", ret);
                NError(-ret).ThrowErr(env);
                return nullptr;
            }
        } else {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
            NError(ret).ThrowErr(env);
            return nullptr;
        }
#else
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
#endif
    }
    auto file = InstantiateFile(env, ret, pathStr, false).val_;
    return file;
}

struct AsyncOpenFileArg {
    int fd;
    string path;
    string uri;
};

static NError AsyncCbExec(shared_ptr<AsyncOpenFileArg> arg, const string &path, unsigned int mode, napi_env env)
{
    string pStr = path;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    Uri uri(path);
    string bundleName = uri.GetAuthority();
    string uriType = uri.GetScheme();
    string uriPath = uri.GetPath();
    if (uriType == SCHEME_FILE) {
        AppFileService::ModuleFileUri::FileUri fileUri(path);
        pStr = fileUri.GetRealPath();
        if ((bundleName == MEDIA || bundleName == DOCS) &&
            access(pStr.c_str(), F_OK) != 0) {
            int ret = OpenFileByDatashare(path, mode);
            if (ret >= 0) {
                arg->fd = ret;
                arg->path = "";
                arg->uri = path;
                return NError(ERRNO_NOERR);
            }
            HILOGE("Failed to open file by Datashare error %{public}d", ret);
            return NError(-ret);
        }
    } else if (uriType == DATASHARE) {
        // datashare:////#fdFromBinder=xx
        int fd = -1;
        if (RemoteUri::IsRemoteUri(path, fd, mode)) {
            if (fd >= 0) {
                arg->fd = fd;
                arg->path = "";
                arg->uri = path;
                return NError(ERRNO_NOERR);
            }
            HILOGE("Failed to open file by RemoteUri");
            return NError(E_INVAL);
        }
    }
#endif
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return NError(ENOMEM);
    }
    int ret = uv_fs_open(nullptr, open_req.get(), pStr.c_str(), mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
        if (bundleName == MEDIA) {
            ret = OpenFileByDatashare(path, mode);
            if (ret < 0) {
                HILOGE("Failed to open file by Datashare error %{public}d", ret);
                return NError(-ret);
            }
        } else {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
            return NError(ret);
        }
#else
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        return NError(ret);
#endif
    }
    arg->fd = ret;
    arg->path = pStr;
    arg->uri = "";
    return NError(ERRNO_NOERR);
}

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
        return nullptr;
    }
    auto arg = CreateSharedPtr<AsyncOpenFileArg>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, path = string(path.get()), mode = mode, env = env]() -> NError {
        return AsyncCbExec(arg, path, mode, env);
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
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
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