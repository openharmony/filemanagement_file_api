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
#include "open_v9.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <uv.h>

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "datashare_helper.h"
#include "remote_uri.h"
#include "ability.h"

#include "../class_file/file_entity.h"
#include "../class_file/file_n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, int> GetJsFlags(napi_env env, const NFuncArg &funcArg)
{
    int mode = O_RDONLY;
    bool succ = false;
    if (funcArg.GetArgc() >= NARG_CNT::TWO && NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_number)) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL, true).ThrowErr(env);
            return { false, mode };
        }
        (void)CommonFunc::ConvertJsFlags(mode);
    }
    return { true, mode };
}

static NVal InstantiateFile(napi_env env, int fd, string path)
{
    napi_value objRAF = NClass::InstantiateClass(env, FileNExporter::className_, {});
    if (!objRAF) {
        UniError(EIO, true).ThrowErr(env);
        return NVal();
    }

    auto rafEntity = NClass::GetEntityOf<FileEntity>(env, objRAF);
    if (!rafEntity) {
        UniError(EIO, true).ThrowErr(env);
        return NVal();
    }
    auto fdg = make_unique<FDGuard>(fd, false);
    rafEntity->fd_.swap(fdg);
    rafEntity->path_ = path;
    rafEntity->uri_ = "";
    return { env, objRAF };
}

static int OpenFileByDatashare(napi_env env, napi_value argv, string path)
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
    int fd = -1;
    sptr<FileIoToken> remote = new IRemoteStub<FileIoToken>();
    if (remote == nullptr) {
        return ENOMEM;
    }

    dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
    Uri uri(path);
    fd = dataShareHelper->OpenFile(uri, MEDIA_FILEMODE_READONLY);
    return fd;
}

napi_value OpenV9::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        return nullptr;
    }
    if (DistributedFS::ModuleRemoteUri::RemoteUri::IsMediaUri(path.get())) {
        auto fd = OpenFileByDatashare(env, funcArg[NARG_POS::FIRST], path.get());
        if (fd >= 0) {
            auto File = InstantiateFile(env, fd, path.get()).val_;
            return File;
        }
        UniError(-1, true).ThrowErr(env);
        return nullptr;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    uv_fs_t open_req;
    int ret = uv_fs_open(loop, &open_req, path.get(), mode, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, NULL);
    if (ret < 0) {
        UniError(errno, true).ThrowErr(env);
        return nullptr;
    }
    auto File = InstantiateFile(env, open_req.result, path.get()).val_;
    uv_fs_req_cleanup(&open_req);
    return File;
}

struct AsyncOpenFileArg {
    int fd;
    string path;
    string uri;
};

napi_value OpenV9::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succPath) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto [succMode, mode] = GetJsFlags(env, funcArg);
    if (!succMode) {
        return nullptr;
    }
    auto arg = make_shared<AsyncOpenFileArg>();
    auto argv = funcArg[NARG_POS::FIRST];
    auto cbExec = [arg, argv, path = string(path.get()), mode = mode](napi_env env) -> UniError {
        if (DistributedFS::ModuleRemoteUri::RemoteUri::IsMediaUri(path)) {
            auto fd = OpenFileByDatashare(env, argv, path);
            if (fd >= 0) {
                arg->fd = fd;
                arg->path = path;
                arg->uri = "";
                return UniError(ERRNO_NOERR);
            }
            return UniError(-1, true);
        }
        uv_loop_s *loop = nullptr;
        napi_get_uv_event_loop(env, &loop);
        uv_fs_t open_req;
        int ret = uv_fs_open(loop, &open_req, path.c_str(), mode, S_IRUSR |
            S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH, NULL);
        if (ret < 0) {
            return UniError(errno, true);
        }
        arg->fd = open_req.result;
        arg->path = path;
        arg->uri = "";
        uv_fs_req_cleanup(&open_req);
        return UniError(ERRNO_NOERR);
    };
    auto cbCompl = [arg](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateFile(env, arg->fd, arg->path);
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
} // namespace DistributedFS
} // namespace OHOS