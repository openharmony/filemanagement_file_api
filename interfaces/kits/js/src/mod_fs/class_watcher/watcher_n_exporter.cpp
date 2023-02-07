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

std::shared_ptr<FileWatcher> WatcherNExporter::watcherPtr_;
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

    watcherPtr_ = make_shared<FileWatcher>();
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
    if (!watcherPtr_->StopNotify(watchEntity->data_)) {
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
    if(!watcherPtr_->StartNotify(watchEntity->data_)) {
         NError(errno).ThrowErr(env);
         return nullptr;
    }

    auto cbExec = [watchEntity]() -> NError {
        watcherPtr_->GetNotifyEvent(watchEntity->data_, WatcherCallback);
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

void WatcherNExporter::WatcherCallback(napi_env env,
                                       napi_ref callback,
                                       const std::string &fileName,
                                       const uint32_t &event)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        HILOGE("WatcherCallback loop is nullptr");
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("WatcherCallback work is nullptr");
        return;
    }

    auto callbackContext = make_unique<JSCallbackContext>();
    callbackContext->env_ = env;
    callbackContext->ref_ = callback;
    callbackContext->fileName_ = fileName;
    callbackContext->event_ = event;
    work->data = callbackContext.release();

    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int stat) {
            if (work == nullptr) {
                HILOGE("uv_queue_work w is nullptr");
                return;
            }

            JSCallbackContext *callbackContext = reinterpret_cast<JSCallbackContext *>(work->data);
            do {
                if (callbackContext == nullptr) {
                    HILOGE("callbackContext is null");
                    break;
                }
                napi_env env = callbackContext->env_;
                napi_value jsCallback = nullptr;
                napi_status status = napi_get_reference_value(env, callbackContext->ref_, &jsCallback);
                if (status != napi_ok) {
                    HILOGE("Create reference fail, status: %{public}d", status);
                    break;
                }
                NVal objn = NVal::CreateObject(env);
                objn.AddProp("fileName", NVal::CreateUTF8String(env, callbackContext->fileName_).val_);
                objn.AddProp("event", NVal::CreateUint32(env, callbackContext->event_).val_);
                napi_value retVal = nullptr;
                status = napi_call_function(env, nullptr, jsCallback, 1, &(objn.val_), &retVal);
                if (status != napi_ok) {
                    HILOGE("CallJs napi_call_function fail, status: %{public}d", status);
                    break;
                }
            } while (0);
            delete callbackContext;
            delete work;
        });
    if (ret != 0) {
        HILOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
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
