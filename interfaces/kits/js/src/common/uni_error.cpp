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

#include "uni_error.h"

#include <cstring>
#include <string>
#include <unordered_map>

#include "log.h"
#include "napi/n_val.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;

static napi_value GenerateBusinessError(napi_env env, int32_t errCode, string errMsg)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_value msg = nullptr;
    code = NVal::CreateInt32(env, errCode).val_;
    msg = NVal::CreateUTF8String(env, errMsg).val_;
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, FILEIO_TAG_ERR_CODE.c_str(), code);
    napi_set_named_property(env, businessError, FILEIO_TAG_ERR_MSG.c_str(), msg);
    return businessError;
}

UniError::UniError() {}

UniError::UniError(ELegacy eLegacy) : errno_(eLegacy), codingSystem_(ERR_CODE_SYSTEM_LEGACY) {}

UniError::UniError(int ePosix) : errno_(ePosix), codingSystem_(ERR_CODE_SYSTEM_POSIX) {}

UniError::UniError(int ePosix, bool throwCode) : errno_(ePosix), codingSystem_(ERR_CODE_SYSTEM_POSIX),
    throwCode_(throwCode) {}

UniError::operator bool() const
{
    return errno_ != ERRNO_NOERR;
}

int UniError::GetErrno(ErrCodeSystem cs)
{
    if (errno_ == ERRNO_NOERR) {
        return ERRNO_NOERR;
    }

    if (cs == codingSystem_) {
        return errno_;
    }

    if (cs == ERR_CODE_SYSTEM_POSIX) {
        // Note that we should support more codes here
        return EINVAL;
    }

    // Note that this shall be done properly
    return ELEGACY_INVAL;
}

void UniError::SetErrno(ELegacy eLegacy)
{
    errno_ = eLegacy;
    codingSystem_ = ERR_CODE_SYSTEM_LEGACY;
}

void UniError::SetErrno(int ePosix)
{
    errno_ = ePosix;
    codingSystem_ = ERR_CODE_SYSTEM_POSIX;
}

std::string UniError::GetDefaultErrstr()
{
    if (codingSystem_ != ERR_CODE_SYSTEM_POSIX && codingSystem_ != ERR_CODE_SYSTEM_LEGACY) {
        return "BUG: Curious coding system";
    }
    return strerror(GetErrno(ERR_CODE_SYSTEM_POSIX));
}

napi_value UniError::GetNapiErr(napi_env env)
{
    int errCode = GetErrno(codingSystem_);
    if (errCode == ERRNO_NOERR) {
        return nullptr;
    }
    if (!throwCode_) {
        return GetNapiErr(env, GetDefaultErrstr());
    }
    int32_t code;
    string msg;
    if (errCodeTable.find(errCode) != errCodeTable.end()) {
        code = errCodeTable.at(errCode).first;
        msg = errCodeTable.at(errCode).second;
    } else {
        code = errCodeTable.at(-1).first;
        msg = errCodeTable.at(-1).second;
    }
    return GenerateBusinessError(env, code, msg);
}

napi_value UniError::GetNapiErr(napi_env env, string errMsg)
{
    napi_value msg = NVal::CreateUTF8String(env, errMsg).val_;
    napi_value res = nullptr;
    napi_status createRes = napi_create_error(env, nullptr, msg, &res);
    if (createRes) {
        HILOGE("Failed to create an exception, msg = %{public}s", errMsg.c_str());
    }
    return res;
}

void UniError::ThrowErr(napi_env env)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    int32_t code;
    string msg;
    napi_status throwStatus = napi_ok;
    if (errCodeTable.find(errno_) != errCodeTable.end()) {
        code = errCodeTable.at(errno_).first;
        msg = errCodeTable.at(errno_).second;
    } else {
        code = errCodeTable.at(-1).first;
        msg = errCodeTable.at(-1).second;
    }
    if (!throwCode_) {
        throwStatus = napi_throw_error(env, nullptr, msg.c_str());
    } else {
        throwStatus = napi_throw(env, GenerateBusinessError(env, code, msg));
    }
    
    if (throwStatus != napi_ok) {
        HILOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, msg.c_str());
    }
}

void UniError::ThrowErr(napi_env env, int code)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    if (errCodeTable.find(code) == errCodeTable.end()) {
        code = -1;
    }
    napi_status throwStatus = napi_throw(env, GenerateBusinessError(env, errCodeTable.at(code).first,
        errCodeTable.at(code).second));
    if (throwStatus != napi_ok) {
        HILOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus,
            errCodeTable.at(code).second.c_str());
    }
}

void UniError::ThrowErr(napi_env env, string errMsg)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    // Note that ace engine cannot throw errors created by napi_create_error so far
    napi_status throwStatus = napi_throw_error(env, nullptr, errMsg.c_str());
    if (throwStatus != napi_ok) {
        HILOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, errMsg.c_str());
    }
}
} // namespace DistributedFS
} // namespace OHOS