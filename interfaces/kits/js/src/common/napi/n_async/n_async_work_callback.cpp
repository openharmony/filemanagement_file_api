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

#include "n_async_work_callback.h"
#include "../../log.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;

NAsyncWorkCallback::NAsyncWorkCallback(napi_env env, NVal thisPtr, NVal cb) : NAsyncWorkFactory(env)
{
    ctx_ = new NAsyncContextCallback(thisPtr, cb);
}

static void CallbackExecute(napi_env env, void *data)
{
    auto ctx = static_cast<NAsyncContextCallback *>(data);
    if (ctx != nullptr && ctx->cbExec_ != nullptr) {
        ctx->err_ = ctx->cbExec_(env);
    }
}

static void CallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    auto ctx = static_cast<NAsyncContextCallback *>(data);
    if (ctx == nullptr) {
        napi_close_handle_scope(env, scope);
        return;
    }

    if (ctx->cbComplete_ != nullptr) {
        ctx->res_ = ctx->cbComplete_(env, ctx->err_);
        ctx->cbComplete_ = nullptr;
    }

    vector<napi_value> argv;
    if (!ctx->res_.TypeIsError(true)) {
        argv = { UniError(ERRNO_NOERR).GetNapiErr(env), ctx->res_.val_ };
    } else {
        argv = { ctx->res_.val_ };
    }

    napi_value global = nullptr;
    napi_value callback = ctx->cb_.Deref(env).val_;
    napi_value tmp = nullptr;
    napi_get_global(env, &global);
    napi_status stat = napi_call_function(env, global, callback, argv.size(), argv.data(), &tmp);
    if (stat != napi_ok) {
        HILOGE("Failed to call function for %{public}d", stat);
    }

    napi_close_handle_scope(env, scope);
    napi_delete_async_work(env, ctx->awork_);
    delete ctx;
}

NVal NAsyncWorkCallback::Schedule(string procedureName, NContextCBExec cbExec, NContextCBComplete cbComplete)
{
    if (!ctx_->cb_ || !ctx_->cb_.Deref(env_).TypeIs(napi_function)) {
        HILOGE("The callback shall be a function");
        UniError(EINVAL).ThrowErr(env_);
        return NVal();
    }

    ctx_->cbExec_ = move(cbExec);
    ctx_->cbComplete_ = move(cbComplete);
    napi_value resource = NVal::CreateUTF8String(env_, procedureName).val_;
    napi_status status =
        napi_create_async_work(env_, nullptr, resource, CallbackExecute, CallbackComplete, ctx_, &ctx_->awork_);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Failed to create async work for %{public}d", status);
        return NVal();
    }

    status = napi_queue_async_work(env_, ctx_->awork_);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Failed to queue async work for %{public}d", status);
        return NVal();
    }

    ctx_ = nullptr; // The ownership of ctx_ has been transferred
    return NVal::CreateUndefined(env_);
}
} // namespace DistributedFS
} // namespace OHOS
