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

#include "watcher_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "../common_func.h"
#include "file_utils.h"
#include "filemgmt_libn.h"
#include "filemgmt_libhilog.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value WatcherNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Failed to get param.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto watcherEntity = CreateUniquePtr<WatcherEntity>();
    if (watcherEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    if (!NClass::SetEntityFor<WatcherEntity>(env, funcArg.GetThisVar(), move(watcherEntity))) {
        HILOGE("Failed to set watcherEntity.");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

napi_value WatcherNExporter::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Failed to get param when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        HILOGE("Failed to get watcherEntity when stop.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int ret = FileWatcher::GetInstance().StopNotify(watchEntity->data_);
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to stopNotify errno:%{public}d", errno);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value WatcherNExporter::Start(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Failed to get param when start.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        HILOGE("Failed to get watcherEntity when start.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int ret = FileWatcher::GetInstance().StartNotify(watchEntity->data_);
    if (ret != ERRNO_NOERR) {
        HILOGE("Failed to startNotify.");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        FileWatcher::GetInstance().GetNotifyEvent(WatcherCallback);
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            HILOGE("Failed to execute complete.");
            return {env, err.GetNapiErr(env)};
        }
        return {NVal::CreateUndefined(env)};
    };

    const string procedureName = "FileIOStartWatcher";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
}

static void WatcherCallbackComplete(uv_work_t *work, int stat)
{
    if (work == nullptr) {
        HILOGE("Failed to get uv_queue_work pointer");
        return;
    }

    WatcherNExporter::JSCallbackContext *callbackContext =
        reinterpret_cast<WatcherNExporter::JSCallbackContext *>(work->data);
    do {
        if (callbackContext == nullptr) {
            HILOGE("Failed to create context pointer");
            break;
        }
        if (!callbackContext->ref_) {
            HILOGE("Failed to get nref reference");
            break;
        }
        napi_handle_scope scope = nullptr;
        napi_status status = napi_open_handle_scope(callbackContext->env_, &scope);
        if (status != napi_ok) {
            HILOGE("Failed to open handle scope, status: %{public}d", status);
            break;
        }
        napi_env env = callbackContext->env_;
        napi_value jsCallback = callbackContext->ref_.Deref(env).val_;
        NVal objn = NVal::CreateObject(env);
        objn.AddProp("fileName", NVal::CreateUTF8String(env, callbackContext->fileName_).val_);
        objn.AddProp("event", NVal::CreateUint32(env, callbackContext->event_).val_);
        objn.AddProp("cookie", NVal::CreateUint32(env, callbackContext->cookie_).val_);
        napi_value retVal = nullptr;
        status = napi_call_function(env, nullptr, jsCallback, 1, &(objn.val_), &retVal);
        if (status != napi_ok) {
            HILOGE("Failed to call napi_call_function, status: %{public}d", status);
        }
        status = napi_close_handle_scope(callbackContext->env_, scope);
        if (status != napi_ok) {
            HILOGE("Failed to close handle scope, status: %{public}d", status);
        }
    } while (0);
    delete callbackContext;
    delete work;
}

void WatcherNExporter::WatcherCallback(napi_env env, NRef &callback, const std::string &fileName,
                                       const uint32_t &event, const uint32_t &cookie)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        HILOGE("Failed to get uv event loop");
        return;
    }
    if (!callback) {
        HILOGE("Failed to parse watcher callback");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("Failed to create uv_work_t pointer");
        return;
    }

    JSCallbackContext *callbackContext = new (std::nothrow) JSCallbackContext(callback);
    if (callbackContext == nullptr) {
        delete work;
        return;
    }
    callbackContext->env_ = env;
    callbackContext->fileName_ = fileName;
    callbackContext->event_ = event;
    callbackContext->cookie_ = cookie;
    work->data = reinterpret_cast<void *>(callbackContext);
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, reinterpret_cast<uv_after_work_cb>(WatcherCallbackComplete));
    if (ret != 0) {
        HILOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete callbackContext;
        delete work;
    }
}

bool WatcherNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("start", Start),
        NVal::DeclareNapiFunction("stop", Stop),
    };

    string className = GetClassName();
    auto [resDefineClass, classValue] =
        NClass::DefineClass(exports_.env_, className, WatcherNExporter::Constructor, std::move(props));
    if (!resDefineClass) {
        HILOGE("Failed to DefineClass");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    bool succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Failed to SaveClass");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string WatcherNExporter::GetClassName()
{
    return WatcherNExporter::className_;
}

WatcherNExporter::WatcherNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

WatcherNExporter::~WatcherNExporter() {}
} // namespace OHOS::FileManagement::ModuleFileIO
