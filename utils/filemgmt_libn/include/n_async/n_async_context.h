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

#ifndef FILEMGMT_LIBN_N_ASYNC_CONTEXT_H
#define FILEMGMT_LIBN_N_ASYNC_CONTEXT_H

#include <functional>

#include "n_error.h"
#include "n_ref.h"
#include "n_val.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using NContextCBExec = std::function<NError()>;
using NContextCBComplete = std::function<NVal(napi_env, NError)>;

class NAsyncContext {
public:
    NError err_;
    NVal res_;
    NContextCBExec cbExec_;
    NContextCBComplete cbComplete_;
    napi_async_work awork_;
    NRef thisPtr_;

    NAsyncContext(NVal thisPtr)
        : err_(0), res_(NVal()), cbExec_(nullptr), cbComplete_(nullptr), awork_(nullptr), thisPtr_(thisPtr)
    {
    }
    virtual ~NAsyncContext() = default;
};

class NAsyncContextPromise : public NAsyncContext {
public:
    napi_deferred deferred_ = nullptr;
    explicit NAsyncContextPromise(NVal thisPtr) : NAsyncContext(thisPtr) {}
    ~NAsyncContextPromise() = default;
};

class NAsyncContextCallback : public NAsyncContext {
public:
    NRef cb_;
    NAsyncContextCallback(NVal thisPtr, NVal cb) : NAsyncContext(thisPtr), cb_(cb) {}
    ~NAsyncContextCallback() = default;
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
#endif // FILEMGMT_LIBN_N_ASYNC_CONTEXT_H