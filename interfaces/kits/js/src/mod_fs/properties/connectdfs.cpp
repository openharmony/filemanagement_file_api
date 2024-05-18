/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "connectdfs.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "distributed_file_daemon_manager.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace fs = std::filesystem;

ConnectDfsCB *ConnectDfs::CreateConnectDfsCBCBInfo(napi_env &env)
{
    HILOGI("CreateConnectDfsCBCBInfo called");
    auto connectDfsCB = new(std::nothrow) ConnectDfsCB;
    if (connectDfsCB == nullptr) {
        HILOGE("CreateConnectDfsCBCBInfo failed, connectDfsCB == nullptr");
        return nullptr;
    }
    connectDfsCB->cbBase.cbInfo.env = env;
    connectDfsCB->cbBase.asyncWork = nullptr;
    connectDfsCB->cbBase.deferred = nullptr;
    connectDfsCB->callbackRef = nullptr;
    HILOGI("CreateConnectDfsCBCBInfo end");
    return connectDfsCB;
}

void cbExec(napi_env env, void *data)
{
    HILOGI("cbExec for connectDfs called");
    auto connectDfsCB = static_cast<ConnectDfsCB *>(data);
    sptr<NAPIDfsListener> dfsListeners(new (std::nothrow) NAPIDfsListener());
    connectDfsCB->jsCallbackObject = dfsListeners;
    connectDfsCB->jsCallbackObject->SetConnectDfsEnv(env);
    HILOGI("connectDfsCB set env success");
    if (connectDfsCB->dfsConnectCB.callback != nullptr) {
        connectDfsCB->jsCallbackObject->
            SetConnectDfsCBRef(connectDfsCB->dfsConnectCB.callback);
        HILOGI("connectDfsCB set callback success");
    } else {
        connectDfsCB->jsCallbackObject->
            SetConnectDfsPromiseRef(connectDfsCB->cbBase.deferred);
        HILOGI("connectDfsCB set promise success");
    }

    connectDfsCB->result = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
        OpenP2PConnectionEx(connectDfsCB->networkId, connectDfsCB->jsCallbackObject);
    HILOGI(" cbExec end ret = %{public}d", connectDfsCB->result);
}

void cbCompl(napi_env env, napi_status status, void *data)
{
    HILOGI("cbCompl for connectDfs called");
    auto connectDfsCB = static_cast<ConnectDfsCB *>(data);
    napi_value result[NARG_CNT::TWO] = { nullptr };
    napi_get_undefined(env, &result[NARG_POS::SECOND]);
    if (connectDfsCB->result == ERRNO_NOERR) {
        napi_get_undefined(env, &result[NARG_POS::FIRST]);
        napi_resolve_deferred(env, connectDfsCB->cbBase.deferred, result[NARG_POS::SECOND]);
    } else {
        result[NARG_POS::FIRST] = NError(connectDfsCB->result).GetNapiErr(env);
        napi_reject_deferred(env, connectDfsCB->cbBase.deferred, result[NARG_POS::FIRST]);
    }
    napi_delete_async_work(env, connectDfsCB->cbBase.asyncWork);
    delete connectDfsCB;
    connectDfsCB = nullptr;
    HILOGI("cbCompl for connectDfs end");
}

tuple<bool, NVal> ConnectDfs::GetListenerFromOptionArg(napi_env env, const NFuncArg &funcArg)
{
    NVal op(env, funcArg[NARG_POS::SECOND]);
    if (!op.HasProp("onStatus") || op.GetProp("onStatus").TypeIs(napi_undefined)) {
        return { true, NVal() };
    }
    NVal onStatus = op.GetProp("onStatus");
    if (!onStatus.TypeIs(napi_function)) {
        HILOGE("Illegal dfsListeners.onStatus type");
        return { false, NVal() };
    }
    return { true, onStatus };
}

tuple<bool, std::string> ConnectDfs::ParseJsOperand(napi_env env, NVal paramFromJsArg)
{
    auto [succ, param, ignore] = paramFromJsArg.ToUTF8String();
    if (!succ) {
        HILOGE("parse parameter failed.");
        return { false, "" };
    }
    std::string paramStr = std::string(param.get());
    return { true, paramStr };
}

int ConnectDfs::ParseJsParam(napi_env env, NFuncArg &funcArg, ConnectDfsCB *connectDfsCB)
{
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        return E_PARAMS;
    }
    auto [succNetworkId, networkId] = ParseJsOperand(env, { env, funcArg[NARG_POS::FIRST] });
    auto [succDfsListeners, dfsListeners] = GetListenerFromOptionArg(env, funcArg);
    if (!succNetworkId || !succDfsListeners) {
        HILOGE("The first/second argument requires string/napi_function");
        return E_PARAMS;
    }
    connectDfsCB->networkId = networkId;
    napi_create_reference(env, dfsListeners.val_, 1, &connectDfsCB->dfsConnectCB.callback);
    return ERRNO_NOERR;
}

napi_value ConnectDfs::Async(napi_env env, napi_callback_info info)
{
    HILOGI("ConnectDfs::Async called");
    ConnectDfsCB *connectDfsCB = CreateConnectDfsCBCBInfo(env);
    if (connectDfsCB == nullptr) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    auto result = ParseJsParam(env, funcArg, connectDfsCB);
    if (result != ERRNO_NOERR) {
        NError(result).ThrowErr(env);
        delete connectDfsCB;
        connectDfsCB = nullptr;
        return nullptr;
    }

    napi_value ret = nullptr;
    napi_status status;
    status = napi_create_promise(env, &connectDfsCB->cbBase.deferred, &ret);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot create promise for %{public}d", status);
        delete connectDfsCB;
        connectDfsCB = nullptr;
        return nullptr;
    }
    
    status = napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_,
        cbExec, cbCompl, static_cast<void *>(connectDfsCB), &connectDfsCB->cbBase.asyncWork);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Failed to create async work for %{public}d", status);
        delete connectDfsCB;
        connectDfsCB = nullptr;
        return nullptr;
    }
    
    status = napi_queue_async_work(env, connectDfsCB->cbBase.asyncWork);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Failed to queue async work for %{public}d", status);
        delete connectDfsCB;
        connectDfsCB = nullptr;
        return nullptr;
    }

    if (ret == nullptr) {
        HILOGE("napi_async_work ret = nullptr");
        NError(E_PARAMS).ThrowErr(env);
        return NVal::CreateUndefined(env).val_;
    }
    HILOGI("ConnectDfs end");
    return ret;
}

ConnectDfsCB *CheckAndGetParameters(uv_work_t *work, napi_handle_scope *scope)
{
    HILOGI("ConnectDfsCB::CheckAndGetParameters GetParam called");
    if (work == nullptr) {
        HILOGE("ConnectDfsCB, GetParam work is null");
        return nullptr;
    }
    ConnectDfsCB *connectDfsCB = static_cast<ConnectDfsCB *>(work->data);
    if (connectDfsCB == nullptr) {
        HILOGE("ConnectDfsCB, GetParam connectDfsCB is null");
        delete work;
        work = nullptr;
        return nullptr;
    }
    napi_open_handle_scope(connectDfsCB->cbBase.cbInfo.env, scope);
    if (scope == nullptr) {
        delete connectDfsCB;
        connectDfsCB = nullptr;
        delete work;
        work = nullptr;
        return nullptr;
    }
    HILOGI("ConnectDfsCB::CheckAndGetParameters GetParam end");
    return connectDfsCB;
}

void NAPIDfsListener::SetConnectDfsEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIDfsListener::SetConnectDfsCBRef(const napi_ref &ref)
{
    onStatusRef_ = ref;
}

void NAPIDfsListener::SetConnectDfsPromiseRef(const napi_deferred &promiseDeferred)
{
    promiseDeferred_ = promiseDeferred;
}

napi_value WrapInt32(napi_env &env, int32_t num, const std::string &paramName)
{
    HILOGI("WrapInt32 called");
    napi_value jsObject = nullptr;
    napi_create_object(env, &jsObject);
    napi_value jsValue = nullptr;
    HILOGD("WrapInt32 called. %{public}s = %{public}d", paramName.c_str(), num);
    napi_create_int32(env, num, &jsValue);
    napi_set_named_property(env, jsObject, paramName.c_str(), jsValue);

    return jsObject;
}

napi_value WrapString(napi_env &env, const std::string &param, const std::string &paramName)
{
    HILOGI("WrapString called");
    napi_value jsValue = nullptr;
    HILOGD("WrapString called. %{public}s = %{public}s", paramName.c_str(), param.c_str());
    napi_create_string_utf8(env, param.c_str(), NAPI_AUTO_LENGTH, &jsValue);

    return jsValue;
}

void UvWorkAfterOnStaus(uv_work_t *work, int status)
{
    HILOGI("UvWorkAfterOnStaus called");
    napi_handle_scope scope = nullptr;
    ConnectDfsCB *connectDfsCB = CheckAndGetParameters(work, &scope);
    if (connectDfsCB == nullptr) {
        return;
    }
    HILOGI("UvWorkAfterOnStaus, status = %{public}d", connectDfsCB->status);
    
    napi_value result[NARG_CNT::TWO] = {nullptr};
    result[NARG_POS::FIRST] = WrapString(connectDfsCB->cbBase.cbInfo.env, connectDfsCB->networkId.c_str(), "networkId");
    result[NARG_POS::SECOND] = WrapInt32(connectDfsCB->cbBase.cbInfo.env, connectDfsCB->status, "status");
    if (connectDfsCB->cbBase.deferred == nullptr) {
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(connectDfsCB->cbBase.cbInfo.env, &undefined);
        napi_value callResult = nullptr;
        napi_get_reference_value(connectDfsCB->cbBase.cbInfo.env,
            connectDfsCB->cbBase.cbInfo.callback, &callback);
        napi_call_function(connectDfsCB->cbBase.cbInfo.env, undefined, callback, 1, result, &callResult);
        if (connectDfsCB->cbBase.cbInfo.callback != nullptr) {
            napi_delete_reference(connectDfsCB->cbBase.cbInfo.env, connectDfsCB->cbBase.cbInfo.callback);
        }
    } else {
        napi_value res[NARG_CNT::TWO] = { nullptr };
        napi_get_undefined(connectDfsCB->cbBase.cbInfo.env, &res[NARG_POS::SECOND]);
        if (connectDfsCB->status == ERRNO_NOERR) {
            napi_resolve_deferred(connectDfsCB->cbBase.cbInfo.env,
                connectDfsCB->cbBase.deferred, res[NARG_POS::SECOND]);
        } else {
            res[NARG_POS::FIRST] = NError(connectDfsCB->status).GetNapiErr(connectDfsCB->cbBase.cbInfo.env);
            napi_reject_deferred(connectDfsCB->cbBase.cbInfo.env, connectDfsCB->cbBase.deferred, res[NARG_POS::FIRST]);
        }
    }
        
    napi_close_handle_scope(connectDfsCB->cbBase.cbInfo.env, scope);
    delete connectDfsCB;
    connectDfsCB = nullptr;
    delete work;
    work = nullptr;
    HILOGI("UvWorkAfterOnStaus end");
}

void NAPIDfsListener::OnStatus(const std::string &networkId, int32_t status)
{
    HILOGI("NAPIDfsListener::OnStatus called");
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOGE("NAPIDfsListener::OnStatus, loop == nullptr");
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }

    auto connectDfsCB = new (std::nothrow) ConnectDfsCB;
    if (connectDfsCB == nullptr) {
        HILOGE("NAPIDfsListener::OnStatus, connectDfsCb == nullptr");
        delete work;
        work = nullptr;
        return;
    }
    connectDfsCB->cbBase.cbInfo.env = env_;
    if (onStatusRef_ != nullptr) {
        connectDfsCB->cbBase.cbInfo.callback = onStatusRef_;
    } else {
        connectDfsCB->cbBase.deferred = promiseDeferred_;
    }
    connectDfsCB->networkId = networkId;
    connectDfsCB->status = status;
    work->data = static_cast<void *>(connectDfsCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkAfterOnStaus);
    if (rev != ERRNO_NOERR) {
        delete connectDfsCB;
        connectDfsCB = nullptr;
        delete work;
        work = nullptr;
    }
    HILOGI("NAPIDfsListener::OnStatus end");
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS