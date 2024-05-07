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
#include "ability_manager_client.h"
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
const std::string SCHEME_BROKER = "content";
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
constexpr uint32_t MAX_WANT_FLAG = 4;
#endif

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

static int OpenFileByPath(const string &path, unsigned int mode)
{
    unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    return ret;
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)

static int OpenFileByDatashare(const string &path, unsigned int flags)
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
    int fd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(flags));
    return fd;
}

static tuple<int, string> OpenByFileDataUri(Uri &uri, const string &uriStr, unsigned int mode)
{
    string bundleName = uri.GetAuthority();
    AppFileService::ModuleFileUri::FileUri fileUri(uriStr);
    string realPath = fileUri.GetRealPath();
    if ((bundleName == MEDIA || bundleName == DOCS) && access(realPath.c_str(), F_OK) != 0) {
        int res = OpenFileByDatashare(uri.ToString(), mode);
        if (res < 0) {
            HILOGE("Failed to open file by Datashare error %{public}d", res);
        }
        return { res, uri.ToString() };
    }
    int ret = OpenFileByPath(realPath, mode);
    if (ret < 0) {
        if (bundleName == MEDIA) {
            ret = OpenFileByDatashare(uriStr, mode);
            if (ret < 0) {
                HILOGE("Failed to open file by Datashare error %{public}d", ret);
            }
        } else {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
        }
    }
    return { ret, uriStr };
}

static tuple<int, string> OpenFileByBroker(const Uri &uri, uint32_t mode)
{
    uint32_t flag = (mode % MAX_WANT_FLAG) > 0 ?
        AAFwk::Want::FLAG_AUTH_WRITE_URI_PERMISSION :
        AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION;
    int ret = AAFwk::AbilityManagerClient::GetInstance()->OpenFile(uri, flag);
    if (ret < 0) {
        HILOGE("Failed to open file by Broker error %{public}d", ret);
    }
    return { ret, uri.ToString() };
}

static tuple<int, string> OpenFileByUri(const string &path, unsigned int mode)
{
    Uri uri(path);
    string uriType = uri.GetScheme();
    if (uriType == SCHEME_FILE) {
        return OpenByFileDataUri(uri, path, mode);
    } else if (uriType == SCHEME_BROKER) {
        return OpenFileByBroker(uri, mode);
    } else if (uriType == DATASHARE) {
        // datashare:////#fdFromBinder=xx
        int fd = -1;
        if (RemoteUri::IsRemoteUri(path, fd, mode)) {
            if (fd >= 0) {
                return { fd, path };
            }
            HILOGE("Failed to open file by RemoteUri");
        }
    }
    HILOGE("Failed to open file by invalid uri");
    return { -EINVAL, path };
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
    string pathStr(path.get());
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (pathStr.find("://") != string::npos) {
        auto [res, uriStr] = OpenFileByUri(pathStr, mode);
        if (res < 0) {
            NError(res).ThrowErr(env);
            return nullptr;
        }
        return InstantiateFile(env, res, uriStr, true).val_;
    }
#endif
    int ret = OpenFileByPath(pathStr, mode);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return InstantiateFile(env, ret, pathStr, false).val_;
}

struct AsyncOpenFileArg {
    int fd;
    string path;
    string uri;
};

static NError AsyncCbExec(shared_ptr<AsyncOpenFileArg> arg, const string &path, unsigned int mode)
{
    string pathStr(path);
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    if (pathStr.find("://") != string::npos) {
        auto [res, uriStr] = OpenFileByUri(pathStr, mode);
        if (res < 0) {
            return NError(res);
        }
        arg->fd = res;
        arg->path = "";
        arg->uri = uriStr;
        return NError(ERRNO_NOERR);
    }
#endif
    int ret = OpenFileByPath(pathStr, mode);
    if (ret < 0) {
        HILOGD("Failed to open file for libuv error %{public}d", ret);
        return NError(ret);
    }
    arg->fd = ret;
    arg->path = pathStr;
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
    auto cbExec = [arg, path = string(path.get()), mode = mode]() -> NError {
        return AsyncCbExec(arg, path, mode);
    };
    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        if (arg->path.empty() && arg->uri.size()) {
            return InstantiateFile(env, arg->fd, arg->uri, true);
        }
        return InstantiateFile(env, arg->fd, arg->path, false);
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