/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

constexpr size_t NAME_THRESH_LONG = 6;
constexpr size_t NAME_THRESH_MID = 4;
constexpr size_t NAME_THRESH_SHORT = 2;
constexpr size_t SHOW_FIRST_LONG = 2;
constexpr size_t SHOW_LAST_LONG = 1;
constexpr size_t SHOW_FIRST_MID = 1;
constexpr size_t SHOW_LAST_MID = 1;
constexpr size_t SHOW_FIRST_SHORT = 1;
constexpr char ANON_CHAR = '*';
constexpr size_t SUFFIX_LEN = 3;
constexpr size_t PATH_MAX_LEN = 4096;
constexpr size_t NAME_MAX_LEN = 256;
constexpr size_t CHAR_LEN_1BYTE = 1;
constexpr size_t CHAR_LEN_2BYTE = 2;
constexpr size_t CHAR_LEN_3BYTE = 3;
constexpr size_t CHAR_LEN_4BYTE = 4;

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
    return NO_ERR_MSG_ERR;
}

static size_t Utf8CharLen(unsigned char lead)
{
    if (lead < 0x80) {
        return CHAR_LEN_1BYTE;
    }
    if ((lead & 0xE0) == 0xC0) {
        return CHAR_LEN_2BYTE;
    }
    if ((lead & 0xF0) == 0xE0) {
        return CHAR_LEN_3BYTE;
    }
    if ((lead & 0xF8) == 0xF0) {
        return CHAR_LEN_4BYTE;
    }
    return CHAR_LEN_1BYTE;
}

static size_t Utf8CharsNum(const std::string& s)
{
    size_t count = 0;
    for (size_t i = 0; i < s.size();) {
        i += Utf8CharLen(static_cast<unsigned char>(s[i]));
        count++;
    }
    return count;
}

static std::string Utf8FirstN(const std::string& s, size_t n)
{
    size_t i = 0;
    for (size_t c = 0; c < n && i < s.size(); c++) {
        i += Utf8CharLen(static_cast<unsigned char>(s[i]));
    }
    return s.substr(0, i);
}

static std::string Utf8LastN(const std::string& s, size_t n)
{
    size_t end = s.size();
    for (size_t c = 0; c < n && end > 0; c++) {
        end--;
        while (end > 0 && (static_cast<unsigned char>(s[end]) & 0xC0) == 0x80) {
            end--;
        }
    }
    return s.substr(end);
}

static std::string TruncateToBytes(const std::string& s, size_t maxBytes)
{
    if (s.size() <= maxBytes) {
        return s;
    }
    size_t i = 0;
    while (i < s.size()) {
        size_t charLen = Utf8CharLen(static_cast<unsigned char>(s[i]));
        if (i + charLen > maxBytes) {
            break;
        }
        i += charLen;
    }
    return s.substr(0, i);
}

static std::string AnonymizeName(const std::string& name)
{
    size_t len = Utf8CharsNum(name);
    if (len >= NAME_THRESH_LONG) {
        return Utf8FirstN(name, SHOW_FIRST_LONG)
             + std::string(len - SHOW_FIRST_LONG - SHOW_LAST_LONG, ANON_CHAR)
             + Utf8LastN(name, SHOW_LAST_LONG);
    } else if (len >= NAME_THRESH_MID) {
        return Utf8FirstN(name, SHOW_FIRST_MID)
             + std::string(len - SHOW_FIRST_MID - SHOW_LAST_MID, ANON_CHAR)
             + Utf8LastN(name, SHOW_LAST_MID);
    } else if (len >= NAME_THRESH_SHORT) {
        return Utf8FirstN(name, SHOW_FIRST_SHORT)
             + std::string(len - SHOW_FIRST_SHORT, ANON_CHAR);
    } else if (len == 1) {
        return std::string(1, ANON_CHAR);
    }
    return name;
}

std::string AnonymizePath(const std::string& path)
{
    if (path.empty()) {
        return "";
    }

    std::string input = TruncateToBytes(path, PATH_MAX_LEN);

    std::string prefix;
    std::string rest = input;
    size_t pos = input.find("://");
    if (pos != std::string::npos) {
        prefix = input.substr(0, pos + SUFFIX_LEN);
        rest = input.substr(pos + SUFFIX_LEN);
    }

    std::vector<std::string> parts;
    size_t start = 0;
    for (size_t i = 0; i <= rest.size(); i++) {
        if (i == rest.size() || rest[i] == '/') {
            parts.push_back(rest.substr(start, i - start));
            start = i + 1;
        }
    }

    std::string result = prefix;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) {
            result += '/';
        }
        std::string p = TruncateToBytes(parts[i], NAME_MAX_LEN);
        bool isLast = (i == parts.size() - 1);
        if (isLast && !p.empty() && p.find('.') != std::string::npos) {
            size_t dot = p.rfind('.');
            std::string before = p.substr(0, dot);
            std::string after = p.substr(dot + 1);

            if (!before.empty()) {
                result += AnonymizeName(before);
            }
            result += '.';
            result += AnonymizeName(after);
        } else {
            result += AnonymizeName(p);
        }
    }

    return result;
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
        errMsg_ = errCodeTable.at(UNKROWN_ERR).second + " Internal errno is " + to_string(abs(errCode));
    }
}

NError::NError(int errCode, const std::string &errMsg)
{
    int genericCode = ConvertUVCode2ErrCode(errCode);
    auto it = errCodeTable.find(genericCode);
    if (it != errCodeTable.end()) {
        errno_ = it->second.first;
        errMsg_ = it->second.second + ". " + errMsg;
    } else {
        errno_ = errCodeTable.at(UNKROWN_ERR).first;
        errMsg_ = errMsg;
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

void NError::ThrowErrWithMsg(napi_env env, const std::string &errMsg)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    napi_status status = napi_throw(env, GenerateBusinessError(env, errno_, std::move(errMsg)));
    if (status != napi_ok) {
        HILOGE("Failed to throw a BusinessError,error code is %{public}d, error message is %{public}s",
            errno_, errMsg.c_str());
    }
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS