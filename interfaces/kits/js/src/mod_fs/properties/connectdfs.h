/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_FILE_API_CONNECTDFS_H
#define FILEMANAGEMENT_FILE_API_CONNECTDFS_H

#include <chrono>
#include <set>
#include <sys/inotify.h>
#include <thread>

#include "bundle_mgr_client_impl.h"
#include "common_func.h"
#include "file_dfs_listener_stub.h"
#include "distributed_file_daemon_manager.h"
#include "filemgmt_libn.h"
#include "n_async/n_ref.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::AppExecFwk;

napi_value WrapInt32(napi_env &env, int32_t num, const std::string &paramName);
napi_value WrapString(napi_env &env, const std::string &param, const std::string &paramName);

struct CallbackInfo {
    napi_env env;
    napi_ref callback;
    napi_deferred deferred;
};

struct CBBase {
    CallbackInfo cbInfo;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
};

struct DfsConnectCB {
    napi_env env;
    napi_ref callback = nullptr;
};

class NAPIDfsListener : public FileDfsListenerStub {
public:
    void OnStatus(const std::string &networkId, int32_t status) override;
    void SetConnectDfsEnv(const napi_env &env);
    void SetConnectDfsCBRef(const napi_ref &ref);
    void SetConnectDfsPromiseRef(const napi_deferred &promiseDeferred);

private:
    napi_env env_ = nullptr;
    napi_ref onStatusRef_ = nullptr;
    napi_deferred promiseDeferred_ = nullptr;
};

struct ConnectDfsCB {
    CBBase cbBase;
    std::string networkId;
    sptr<NAPIDfsListener> jsCallbackObject;
    DfsConnectCB dfsConnectCB;
    int result = 0;
    int status = 0;
    napi_ref callbackRef;
};

ConnectDfsCB *CheckAndGetParameters(ConnectDfsCB *connectDfsCB, napi_handle_scope *scope);

class ConnectDfs final {
public:
    static napi_value Async(napi_env env, napi_callback_info info);

private:
    static tuple<bool, std::string> ParseJsOperand(napi_env env, NVal paramFromJsArg);
    static tuple<bool, NVal> GetListenerFromOptionArg(napi_env env, const NFuncArg &funcArg);
    static int ParseJsParam(napi_env env, NFuncArg &funcArg, ConnectDfsCB *connectDfsCB);
    static ConnectDfsCB *CreateConnectDfsCBCBInfo(napi_env &env);
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_CONNECTDFS_H