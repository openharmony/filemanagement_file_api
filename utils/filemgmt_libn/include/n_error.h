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

#ifndef FILEMGMT_LIBN_N_ERROR_H
#define FILEMGMT_LIBN_N_ERROR_H

#include <functional>
#include <string>
#include <tuple>

#include "n_napi.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
constexpr int ERRNO_NOERR = 0;

class NError {
public:
    NError();
    NError(int ePosix);
    NError(std::function<std::tuple<uint32_t, std::string>()> errGen);
    ~NError() = default;
    explicit operator bool() const;
    napi_value GetNapiErr(napi_env env);
    void ThrowErr(napi_env env);
    void ThrowErr(napi_env env, std::string errMsg);

private:
    int errno_ = ERRNO_NOERR;
    std::string errMsg_;
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_ERROR_H