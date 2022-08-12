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

#include "n_ref.h"

namespace OHOS {
namespace DistributedFS {
NRef::NRef() {}

NRef::NRef(NVal val)
{
    if (val) {
        env_ = val.env_;
        napi_create_reference(val.env_, val.val_, 1, &ref_);
    }
}

NRef::~NRef()
{
    if (ref_) {
        napi_delete_reference(env_, ref_);
    }
}

NRef::operator bool() const
{
    return ref_ != nullptr;
}

NVal NRef::Deref(napi_env env)
{
    if (!ref_) {
        return NVal();
    }

    napi_value val = nullptr;
    napi_get_reference_value(env, ref_, &val);
    return { env, val };
}
} // namespace DistributedFS
} // namespace OHOS
