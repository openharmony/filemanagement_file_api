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

#include "filemgmt_libhilog.h"
#include "n_val.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
using namespace std;

static napi_value GenerateBusinessError(napi_env env, int32_t errCode, string errMsg)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_value msg = nullptr;
    napi_status status;
    code = NVal::CreateInt32(env, errCode).val_;
    msg = NVal::CreateUTF8String(env, errMsg).val_;
    status = napi_create_error(env, nullptr, msg, &businessError);
    if (status != napi_ok) {
        HILOGE("Failed to create a BusinessError, error message is %{public}s", errMsg.c_str());
        return nullptr;
    }
    status = napi_set_named_property(env, businessError, FILEIO_TAG_ERR_CODE.c_str(), code);
    if (status != napi_ok) {
        HILOGE("Failed to set code property on Error, error message is %{public}s", errMsg.c_str());
        return nullptr;
    }
    return businessError;
}

static int ConvertUVCode2ErrCode(int errCode)
{
    if (errCode >= 0) {
        return errCode;
    }
    auto uvCode = string_view(uv_err_name(errCode));
    if (uvCode2ErrCodeTable.find(uvCode) != uvCode2ErrCodeTable.end()) {
        return uvCode2ErrCodeTable.at(uvCode);
    }
    return UNKROWN_ERR;
}

NError::NError() {}

NError::NError(int errCode)
{
    int genericCode = ConvertUVCode2ErrCode(errCode);
    auto it = errCodeTable.find(genericCode);
    if (it != errCodeTable.end()) {
        errno_ = it->second.first;
        errMsg_ = it->second.second;
    } else {
        errno_ = errCodeTable.at(UNKROWN_ERR).first;
        errMsg_ = errCodeTable.at(UNKROWN_ERR).second + ", errno is " + to_string(abs(errCode));
    }
}

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
    if (errno_ == ERRNO_NOERR) {
        return nullptr;
    }
    return GenerateBusinessError(env, errno_, errMsg_);
}

napi_value NError::GetNapiErr(napi_env env, int errCode)
{
    if (errCode == ERRNO_NOERR) {
        return nullptr;
    }
    int32_t code = 0;
    string msg;
    auto it = errCodeTable.find(errCode);
    if (it != errCodeTable.end()) {
        code = it->second.first;
        msg = it->second.second;
    } else {
        code = errCodeTable.at(UNKROWN_ERR).first;
        msg = errCodeTable.at(UNKROWN_ERR).second;
    }
    errno_ = code;
    errMsg_ = msg;
    return GenerateBusinessError(env, code, msg);
}

napi_value NError::GetNapiErrAddData(napi_env env, int errCode, napi_value data)
{
    if (errCode == ERRNO_NOERR) {
        return nullptr;
    }
    int32_t code = 0;
    string msg;
    if (errCodeTable.find(errCode) != errCodeTable.end()) {
        code = errCodeTable.at(errCode).first;
        msg = errCodeTable.at(errCode).second;
    } else {
        code = errCodeTable.at(UNKROWN_ERR).first;
        msg = errCodeTable.at(UNKROWN_ERR).second;
    }
    errno_ = code;
    errMsg_ = msg;
    napi_value businessError = GenerateBusinessError(env, code, msg);
    napi_status status = napi_set_named_property(env, businessError, FILEIO_TAG_ERR_DATA.c_str(), data);
    if (status != napi_ok) {
        HILOGE("Failed to set data property on Error, error message is %{public}s", msg.c_str());
        return nullptr;
    }
    return businessError;
}

void NError::ThrowErr(napi_env env, int errCode)
{
    int32_t code = 0;
    string msg;
    if (errCodeTable.find(errCode) != errCodeTable.end()) {
        code = errCodeTable.at(errCode).first;
        msg = errCodeTable.at(errCode).second;
    } else {
        code = errCodeTable.at(UNKROWN_ERR).first;
        msg = errCodeTable.at(UNKROWN_ERR).second;
    }
    errno_ = code;
    errMsg_ = msg;
    napi_status status = napi_throw(env, GenerateBusinessError(env, code, msg));
    if (status != napi_ok) {
        HILOGE("Failed to throw a BusinessError, error message is %{public}s", msg.c_str());
    }
}

void NError::ThrowErr(napi_env env, string errMsg)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    // Note that ace engine cannot thow errors created by napi_create_error so far
    napi_status throwStatus = napi_throw_error(env, nullptr, errMsg.c_str());
    if (throwStatus != napi_ok) {
        HILOGE("Failed to throw an Error, error message is %{public}s", errMsg.c_str());
    }
}

void NError::ThrowErrAddData(napi_env env, int errCode, napi_value data)
{
    int32_t code = 0;
    string msg;
    if (errCodeTable.find(errCode) != errCodeTable.end()) {
        code = errCodeTable.at(errCode).first;
        msg = errCodeTable.at(errCode).second;
    } else {
        code = errCodeTable.at(UNKROWN_ERR).first;
        msg = errCodeTable.at(UNKROWN_ERR).second;
    }
    errno_ = code;
    errMsg_ = msg;
    napi_value businessError = GenerateBusinessError(env, code, msg);
    napi_status status = napi_set_named_property(env, businessError, FILEIO_TAG_ERR_DATA.c_str(), data);
    if (status != napi_ok) {
        HILOGE("Failed to set data property on Error, error message is %{public}s", msg.c_str());
        return;
    }
    status = napi_throw(env, businessError);
    if (status != napi_ok) {
        HILOGE("Failed to throw a BusinessError, error message is %{public}s", msg.c_str());
        return;
    }
}

void NError::ThrowErr(napi_env env)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    napi_status status = napi_throw(env, GenerateBusinessError(env, errno_, errMsg_));
    if (status != napi_ok) {
        HILOGE("Failed to throw a BusinessError, error message is %{public}s", errMsg_.c_str());
    }
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS