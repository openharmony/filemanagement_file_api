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

#include "n_error.h"

#include <cstring>

#include "n_val.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;

NError::NError() {}

NError::NError(int ePosix) : errno_(ePosix), errMsg_(strerror(errno_)) {}

NError::NError(std::function<std::tuple<uint32_t, std::string>()> errGen)
{
    tie(errno_, errMsg_) = errGen();
}

NError::operator bool() const
{
    return errno_ != ERRNO_NOERR;
}

napi_value NError::GetNapiErr(napi_env env)
{
    napi_value code = NVal::CreateUTF8String(env, to_string(errno_)).val_;
    napi_value msg = NVal::CreateUTF8String(env, errMsg_).val_;

    napi_value res = nullptr;
    napi_status createRes = napi_create_error(env, code, msg, &res);
    if (createRes) {
        HILOGE("Failed to create an exception, msg = %{public}s", errMsg_.c_str());
    }
    return res;
}

void NError::ThrowErr(napi_env env, string errMsg)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    // Note that ace engine cannot thow errors created by napi_create_error so far
    napi_status throwStatus = napi_throw_error(env, nullptr, errMsg.c_str());
    if (throwStatus != napi_ok) {
        HILOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, errMsg.c_str());
    }
}

void NError::ThrowErr(napi_env env)
{
    ThrowErr(env, errMsg_);
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS