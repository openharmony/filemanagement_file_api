/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
    auto callbackContext = make_unique<JSCallbackContext>();
    callbackContext->env_ = env;
    callbackContext->ref_ = callback;
    callbackContext->fileName_ = fileName;
    callbackContext->event_ = event;
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "FileIOCreateWatcher", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource, [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {
            // JSCallbackData* jsCallbackData = (JSCallbackData*)data;
            auto context = static_cast<JSCallbackContext *>(data);
            napi_value jsCallback = nullptr;
            napi_get_reference_value(env, context->ref_, &jsCallback);
            NVal objn = NVal::CreateObject(env);
            objn.AddProp("fileName", NVal::CreateUTF8String(env, context->fileName_).val_);
            objn.AddProp("event", NVal::CreateUint32(env, context->event_).val_);
            napi_value global = nullptr;
            napi_get_global(env, &global);
            napi_value jsObj = nullptr;
            napi_call_function(env, global, jsCallback, 1, &(objn.val_), &jsObj);
            napi_delete_async_work(env, context->work_);
        },
        static_cast<void *>(callbackContext.get()), &callbackContext->work_);
    napi_queue_async_work(env, callbackContext->work_);
    callbackContext.release();
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
