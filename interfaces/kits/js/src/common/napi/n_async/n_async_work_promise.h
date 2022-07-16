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

#ifndef N_ASYNC_WORK_PROMISE_H
#define N_ASYNC_WORK_PROMISE_H

#include "n_async_work_factory.h"

namespace OHOS {
namespace DistributedFS {
class NAsyncWorkPromise : public NAsyncWorkFactory {
public:
    NAsyncWorkPromise(napi_env env, NVal thisPtr);
    ~NAsyncWorkPromise() = default;

    NVal Schedule(std::string procedureName, NContextCBExec cbExec, NContextCBComplete cbComplete) final;

private:
    NAsyncContextPromise *ctx_;
};
} // namespace DistributedFS
} // namespace OHOS
#endif // N_ASYNC_WORK_PROMISE_H