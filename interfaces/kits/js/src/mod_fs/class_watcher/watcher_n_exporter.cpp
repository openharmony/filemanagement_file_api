/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "filemgmt_libn.h"
#include "filemgmt_libhilog.h"
#include "../common_func.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

std::unique_ptr<FileWatcher> WatcherNExporter::watcherPtr_ = nullptr;
napi_value WatcherNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    unique_ptr<WatcherEntity> watcherEntity = make_unique<WatcherEntity>();
    if (!NClass::SetEntityFor<WatcherEntity>(env, funcArg.GetThisVar(), move(watcherEntity))) {
        NError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj stat");
        return nullptr;
    }

    watcherPtr_ = make_unique<FileWatcher>();
    return funcArg.GetThisVar();
}

napi_value WatcherNExporter::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        NError(EINVAL).ThrowErr(env, "get watcherEntity fail");
        return nullptr;
    }
    if (!watcherPtr_->StopNotify(*(watchEntity->data_))) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    
    return NVal::CreateUndefined(env).val_;
}

napi_value WatcherNExporter::Start(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto watchEntity = NClass::GetEntityOf<WatcherEntity>(env, funcArg.GetThisVar());
    if (!watchEntity) {
        NError(EINVAL).ThrowErr(env, "get watcherEntity fail");
        return nullptr;
    }

    if (!watcherPtr_->StartNotify(*(watchEntity->data_))) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [watchEntity]() -> NError {
        watcherPtr_->GetNotifyEvent(*(watchEntity->data_), WatcherCallback);
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
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
        napi_env env = callbackContext->env_;
        napi_value jsCallback = callbackContext->ref_.Deref(env).val_;

        napi_valuetype valueType;
        napi_typeof(env, jsCallback, &valueType);

        NVal objn = NVal::CreateObject(env);
        objn.AddProp("fileName", NVal::CreateUTF8String(env, callbackContext->fileName_).val_);
        objn.AddProp("event", NVal::CreateUint32(env, callbackContext->event_).val_);
        napi_value retVal = nullptr;
        napi_status status = napi_call_function(env, nullptr, jsCallback, 1, &(objn.val_), &retVal);
        if (status != napi_ok) {
            HILOGE("Failed to call napi_call_function, status: %{public}d", status);
            break;
        }
    } while (0);
    delete callbackContext;
    delete work;
}

void WatcherNExporter::WatcherCallback(napi_env env, NRef &callback, const std::string &fileName, const uint32_t &event)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        HILOGE("Failed to get uv event loop");
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("Failed to create uv_work_t pointer");
        return;
    }

    if (!callback) {
        HILOGE("Failed to pass watcher callback");
        return;
    }

    JSCallbackContext *callbackContext = new (std::nothrow) JSCallbackContext(callback);
    callbackContext->env_ = env;
    callbackContext->fileName_ = fileName;
    callbackContext->event_ = event;
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
        NError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }

    bool succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
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
