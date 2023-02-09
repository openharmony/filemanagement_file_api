/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "common_func.h"
#include "datashare_helper.h"
#include "filemgmt_libhilog.h"
#include "remote_uri.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

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

static NVal InstantiateFile(napi_env env, int fd, string pathOrUri, bool isUri)
{
    napi_value objFile = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objFile) {
        HILOGE("Failed to instantiate class");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    auto fileEntity = NClass::GetEntityOf<FileEntity>(env, objFile);
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        NError(EIO).ThrowErr(env);
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

static int OpenFileByDatashare(napi_env env, napi_value argv, string path, unsigned int flags)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    int fd = -1;
    sptr<FileIoToken> remote = new (std::nothrow) IRemoteStub<FileIoToken>();
    if (!remote) {
        return ENOMEM;
    }

    dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
    Uri uri(path);
    fd = dataShareHelper->OpenFile(uri, CommonFunc::GetModeFromFlags(flags));
    return fd;
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
    if (DistributedFS::ModuleRemoteUri::RemoteUri::IsMediaUri(path.get())) {
        auto fd = OpenFileByDatashare(env, funcArg[NARG_POS::FIRST], path.get(), mode);
        if (fd >= 0) {
            auto file = InstantiateFile(env, fd, path.get(), true).val_;
            return file;
        }
        HILOGE("Failed to open file by Datashare");
        NError(-1).ThrowErr(env);
        return nullptr;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
        new uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!open_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    int ret = uv_fs_open(nullptr, open_req.get(), path.get(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
    if (ret < 0) {
        HILOGE("Failed to open file for libuv error %{public}d", ret);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    auto file = InstantiateFile(env, ret, path.get(), false).val_;
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
        if (DistributedFS::ModuleRemoteUri::RemoteUri::IsMediaUri(path)) {
            auto fd = OpenFileByDatashare(env, argv, path, mode);
            if (fd >= 0) {
                arg->fd = fd;
                arg->path = "";
                arg->uri = path;
                return NError(ERRNO_NOERR);
            }
            HILOGE("Failed to open file by Datashare");
            return NError(-1);
        }
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ERRNO_NOERR);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), path.c_str(), mode, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open file for libuv error %{public}d", ret);
            return NError(errno);
        }
        arg->fd = ret;
        arg->path = path;
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