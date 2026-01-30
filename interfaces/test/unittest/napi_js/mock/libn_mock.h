/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_LIBN_MOCK_H
#define INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_LIBN_MOCK_H

#include <gmock/gmock.h>

#include "filemgmt_libn.h"
#include "n_napi.h"

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
    virtual napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result) = 0;
    virtual napi_status napi_create_array(napi_env env, napi_value *result) = 0;
    virtual napi_status napi_delete_reference(napi_env env, napi_ref ref) = 0;
    virtual napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) = 0;
    virtual napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype *result) = 0;
    virtual napi_value InstantiateClass(
        napi_env env, const std::string &className, const std::vector<napi_value> &args) = 0;

    // n_error
    virtual void ThrowErr(napi_env env) = 0;
    virtual void ThrowErr(napi_env env, int errCode) = 0;
    virtual void ThrowErr(napi_env env, std::string errMsg) = 0;
    virtual void ThrowErrAddData(napi_env env, int errCode, napi_value data) = 0;
    virtual void ThrowErrWithMsg(napi_env env, const std::string &errMsg) = 0;

    // n_val
    virtual std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String() = 0;
    virtual std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String(std::string defaultValue) = 0;
    virtual std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8StringPath() = 0;
    virtual std::tuple<bool, bool> ToBool() = 0;
    virtual std::tuple<bool, bool> ToBool(bool defaultValue) = 0;
    virtual std::tuple<bool, int32_t> ToInt32() = 0;
    virtual std::tuple<bool, int32_t> ToInt32(int32_t defaultValue) = 0;
    virtual std::tuple<bool, int64_t> ToInt64() = 0;
    virtual std::tuple<bool, int64_t> ToInt64(int64_t defaultValue) = 0;
    virtual std::tuple<bool, double> ToDouble() = 0;

    virtual bool HasProp(std::string propName) = 0;
    virtual NVal GetProp(std::string propName) = 0;
    virtual bool TypeIs(napi_valuetype expType) = 0;
    virtual NVal CreateUndefined(napi_env env) = 0;
    virtual NVal CreateUTF8String(napi_env env, std::string str) = 0;
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
    MOCK_METHOD(napi_status, napi_remove_wrap, (napi_env, napi_value, void **), (override));
    MOCK_METHOD(napi_status, napi_create_array, (napi_env, napi_value *), (override));
    MOCK_METHOD(napi_status, napi_delete_reference, (napi_env, napi_ref), (override));
    MOCK_METHOD(napi_status, napi_get_reference_value, (napi_env, napi_ref, napi_value *), (override));
    MOCK_METHOD(napi_status, napi_typeof, (napi_env, napi_value, napi_valuetype *), (override));
    MOCK_METHOD(
        napi_value, InstantiateClass, (napi_env, const std::string&, const std::vector<napi_value>&), (override));

    MOCK_METHOD(void, ThrowErr, (napi_env), (override));
    MOCK_METHOD(void, ThrowErr, (napi_env, int), (override));
    MOCK_METHOD(void, ThrowErr, (napi_env, std::string), (override));
    MOCK_METHOD(void, ThrowErrAddData, (napi_env, int, napi_value), (override));
    MOCK_METHOD(void, ThrowErrWithMsg, (napi_env, const std::string &errMsg), (override));

    MOCK_METHOD((std::tuple<bool, std::unique_ptr<char[]>, size_t>), ToUTF8String, (), (override));
    MOCK_METHOD((std::tuple<bool, std::unique_ptr<char[]>, size_t>), ToUTF8String, (std::string), (override));
    MOCK_METHOD((std::tuple<bool, std::unique_ptr<char[]>, size_t>), ToUTF8StringPath, (), (override));
    MOCK_METHOD((std::tuple<bool, bool>), ToBool, (), (override));
    MOCK_METHOD((std::tuple<bool, bool>), ToBool, (bool), (override));
    MOCK_METHOD((std::tuple<bool, int32_t>), ToInt32, (), (override));
    MOCK_METHOD((std::tuple<bool, int32_t>), ToInt32, (int32_t), (override));
    MOCK_METHOD((std::tuple<bool, int64_t>), ToInt64, (), (override));
    MOCK_METHOD((std::tuple<bool, int64_t>), ToInt64, (int64_t), (override));
    MOCK_METHOD((std::tuple<bool, double>), ToDouble, (), (override));

    MOCK_METHOD(bool, HasProp, (std::string), (override));
    MOCK_METHOD(NVal, GetProp, (std::string), (override));
    MOCK_METHOD(bool, TypeIs, (napi_valuetype), (override));
    MOCK_METHOD(NVal, CreateUndefined, (napi_env), (override));
    MOCK_METHOD(NVal, CreateUTF8String, (napi_env, std::string), (override));

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
#endif // INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_LIBN_MOCK_H