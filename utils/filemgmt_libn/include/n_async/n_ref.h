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

#ifndef FILEMGMT_LIBN_N_REF_H
#define FILEMGMT_LIBN_N_REF_H

#include "n_val.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
class NRef {
public:
    NRef();
    NRef(NVal val);
    ~NRef();
    explicit operator bool() const;
    NVal Deref(napi_env env);

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_REF_H