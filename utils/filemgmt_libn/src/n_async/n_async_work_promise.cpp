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

#include "n_async_work_promise.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;

NAsyncWorkPromise::NAsyncWorkPromise(napi_env env, NVal thisPtr) : NAsyncWork(env)
{
    ctx_ = new NAsyncContextPromise(thisPtr);
}

static void PromiseOnExec(napi_env env, void *data)
{
    auto ctx = static_cast<NAsyncContextPromise *>(data);
    if (ctx != nullptr && ctx->cbExec_ != nullptr) {
        ctx->err_ = ctx->cbExec_();
    }
}

static void PromiseOnComplete(napi_env env, napi_status status, void *data)
{
    auto ctx = static_cast<NAsyncContextPromise *>(data);
    if (ctx == nullptr) {
        return;
    }
    if (ctx->cbComplete_ != nullptr) {
        ctx->res_ = ctx->cbComplete_(env, ctx->err_);
    }
    if (!ctx->res_.TypeIsError(true)) {
        napi_status status = napi_resolve_deferred(env, ctx->deferred_, ctx->res_.val_);
        if (status != napi_ok) {
            HILOGE("Internal BUG, cannot resolve promise for %{public}d", status);
        }
    } else {
        napi_status status = napi_reject_deferred(env, ctx->deferred_, ctx->res_.val_);
        if (status != napi_ok) {
            HILOGE("Internal BUG, cannot reject promise for %{public}d", status);
        }
    }
    ctx->deferred_ = nullptr;
    napi_delete_async_work(env, ctx->awork_);
    delete ctx;
}

NVal NAsyncWorkPromise::Schedule(string procedureName, NContextCBExec cbExec, NContextCBComplete cbComplete)
{
    ctx_->cbExec_ = move(cbExec);
    ctx_->cbComplete_ = move(cbComplete);

    napi_status status;
    napi_value result = nullptr;
    status = napi_create_promise(env_, &ctx_->deferred_, &result);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot create promise for %{public}d", status);
        return NVal();
    }

    napi_value resource = NVal::CreateUTF8String(env_, procedureName).val_;
    status = napi_create_async_work(env_, nullptr, resource, PromiseOnExec, PromiseOnComplete, ctx_, &ctx_->awork_);
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
    return {env_, result};
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
