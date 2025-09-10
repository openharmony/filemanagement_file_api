/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_NAPI_MOCK_H
#define INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_NAPI_MOCK_H

#include "n_napi.h"
#include "filemgmt_libn.h"

#include <cstdio>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;

class ILibnMock {
public:
    virtual ~ILibnMock() = default;

    // n_func_arg
    virtual bool InitArgs(std::function<bool()> argcChecker) = 0;
    virtual bool InitArgs(size_t argc) = 0;
    virtual bool InitArgs(size_t minArgc, size_t maxArgc) = 0;
    virtual size_t GetArgc() = 0;
    virtual napi_value GetArg(size_t argPos) = 0;
    virtual napi_value GetThisVar() = 0;

    // n_class GetEntityOf
    virtual napi_status napi_unwrap(napi_env env, napi_value js_object, void **result) = 0;

    // n_error
    virtual void ThrowErr(napi_env env) = 0;
    virtual void ThrowErr(napi_env env, int errCode) = 0;
    virtual void ThrowErr(napi_env env, std::string errMsg) = 0;
    virtual void ThrowErrAddData(napi_env env, int errCode, napi_value data) = 0;

    // n_val
    virtual std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String() = 0;
    virtual std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String(std::string defaultValue) = 0;
    virtual std::tuple<bool, bool> ToBool() = 0;
    virtual std::tuple<bool, bool> ToBool(bool defaultValue) = 0;
    virtual std::tuple<bool, int32_t> ToInt32() = 0;
    virtual std::tuple<bool, int32_t> ToInt32(int32_t defaultValue) = 0;
    virtual std::tuple<bool, int64_t> ToInt64() = 0;
    virtual std::tuple<bool, int64_t> ToInt64(int64_t defaultValue) = 0;
    virtual std::tuple<bool, double> ToDouble() = 0;
};

class LibnMock : public ILibnMock {
public:
    MOCK_METHOD(bool, InitArgs, (std::function<bool()>), (override));
    MOCK_METHOD(bool, InitArgs, (size_t), (override));
    MOCK_METHOD(bool, InitArgs, (size_t, size_t), (override));
    MOCK_METHOD(size_t, GetArgc, (), (override));
    MOCK_METHOD(napi_value, GetArg, (size_t), (override));
    MOCK_METHOD(napi_value, GetThisVar, (), (override));

    MOCK_METHOD(napi_status, napi_unwrap, (napi_env, napi_value, void **), (override));

    MOCK_METHOD(void, ThrowErr, (napi_env), (override));
    MOCK_METHOD(void, ThrowErr, (napi_env, int), (override));
    MOCK_METHOD(void, ThrowErr, (napi_env, std::string), (override));
    MOCK_METHOD(void, ThrowErrAddData, (napi_env, int, napi_value), (override));

    MOCK_METHOD((std::tuple<bool, std::unique_ptr<char[]>, size_t>), ToUTF8String, (), (override));
    MOCK_METHOD((std::tuple<bool, std::unique_ptr<char[]>, size_t>), ToUTF8String, (std::string), (override));
    MOCK_METHOD((std::tuple<bool, bool>), ToBool, (), (override));
    MOCK_METHOD((std::tuple<bool, bool>), ToBool, (bool), (override));
    MOCK_METHOD((std::tuple<bool, int32_t>), ToInt32, (), (override));
    MOCK_METHOD((std::tuple<bool, int32_t>), ToInt32, (int32_t), (override));
    MOCK_METHOD((std::tuple<bool, int64_t>), ToInt64, (), (override));
    MOCK_METHOD((std::tuple<bool, int64_t>), ToInt64, (int64_t), (override));
    MOCK_METHOD((std::tuple<bool, double>), ToDouble, (), (override));

public:
    static std::shared_ptr<LibnMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<LibnMock> libnMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_NAPI_JS_MOD_FS_MOCK_NAPI_MOCK_H