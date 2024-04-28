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

#include "n_async_work_callback.h"

#include <memory>
#include <new>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;

NAsyncWorkCallback::NAsyncWorkCallback(napi_env env, NVal thisPtr, NVal cb) : NAsyncWork(env)
{
    ctx_ = new NAsyncContextCallback(thisPtr, cb);
}

NAsyncWorkCallback::~NAsyncWorkCallback()
{
    if (!ctx_) {
        return;
    }

    unique_ptr<NAsyncContextCallback> ptr(ctx_);
    uv_loop_s *loop = nullptr;
    napi_status status = napi_get_uv_event_loop(env_, &loop);
    if (status != napi_ok) {
        HILOGE("Failed to get uv event loop");
        return;
    }

    auto work = CreateUniquePtr<uv_work_t>();
    if (work == nullptr) {
        HILOGE("Failed to new uv_work_t");
        return;
    }
    work->data = static_cast<void *>(ctx_);

    int ret = uv_queue_work(
        loop, work.get(), [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            NAsyncContextCallback *ctx = static_cast<NAsyncContextCallback *>(work->data);
            delete ctx;
            delete work;
        });
    if (ret) {
        HILOGE("Failed to call uv_queue_work %{public}d", status);
        return;
    }
    ptr.release();
    work.release();
    ctx_ = nullptr;
}

NAsyncWorkCallback::operator bool() const
{
    return bool(ctx_->cb_);
}

static void CallbackExecute(napi_env env, void *data)
{
    auto ctx = static_cast<NAsyncContextCallback *>(data);
    if (ctx != nullptr && ctx->cbExec_ != nullptr) {
        ctx->err_ = ctx->cbExec_();
    } else {
        HILOGE("Callback execute function, This pointer or function address is empty");
    }
}

static void CallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    auto ctx = static_cast<NAsyncContextCallback *>(data);
    if (ctx == nullptr) {
        HILOGE("This pointer address is empty");
        napi_close_handle_scope(env, scope);
        return;
    }
    if (ctx->cbComplete_ != nullptr) {
        ctx->res_ = ctx->cbComplete_(env, ctx->err_);
        ctx->cbComplete_ = nullptr;
    }

    vector<napi_value> argv;
    if (!ctx->res_.TypeIsError(true)) {
        argv = {NError(ERRNO_NOERR).GetNapiErr(env), ctx->res_.val_};
    } else {
        argv = {ctx->res_.val_};
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
        HILOGE("The callback should be a function");
        NError(EINVAL).ThrowErr(env_);
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

static void AfterWorkCallback(napi_env env, napi_status status, void *data, NContextCBComplete cbComplete)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    auto ctx = static_cast<NAsyncContextCallback *>(data);
    if (ctx == nullptr) {
        HILOGE("This pointer address is empty");
        napi_close_handle_scope(env, scope);
        return;
    }
    if (cbComplete) {
        ctx->res_ = cbComplete(env, ctx->err_);
        ctx->cbComplete_ = nullptr;
    }

    vector<napi_value> argv;
    if (!ctx->res_.TypeIsError(true)) {
        argv = {NError(ERRNO_NOERR).GetNapiErr(env), ctx->res_.val_};
    } else {
        argv = {ctx->res_.val_};
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
}

void NAsyncWorkCallback::ThreadSafeSchedule(NContextCBComplete cbComplete)
{
    ctx_->cbExec_ = nullptr;
    ctx_->cbComplete_ = nullptr;

    uv_loop_s *loop = nullptr;
    napi_status status = napi_get_uv_event_loop(env_, &loop);
    if (status != napi_ok) {
        HILOGE("Failed to get uv event loop");
        return;
    }

    auto work = CreateUniquePtr<uv_work_t>();
    if (!work) {
        HILOGE("Failed to new uv_work_t");
        return;
    }

    struct WorkArgs {
        NAsyncWorkCallback *ptr = nullptr;
        NContextCBComplete cb;
    };
    auto workArgs = make_unique<WorkArgs>();
    workArgs->ptr = this;
    workArgs->cb = cbComplete;

    work->data = static_cast<void *>(workArgs.get());

    int ret = uv_queue_work(
        loop, work.get(), [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            auto workArgs = static_cast<WorkArgs *>(work->data);
            AfterWorkCallback(workArgs->ptr->env_, napi_ok, workArgs->ptr->ctx_, workArgs->cb);
            delete workArgs;
            delete work;
        });
    if (ret) {
        HILOGE("Failed to call uv_queue_work %{public}d", status);
        workArgs.reset();
        work.reset();
        return;
    }
    workArgs.release();
    work.release();
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
