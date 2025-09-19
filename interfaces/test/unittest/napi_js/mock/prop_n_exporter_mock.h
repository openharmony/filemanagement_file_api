/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef INTERFACES_TEST_UNITTEST_PROPNEXPORTER_MOCK_H
#define INTERFACES_TEST_UNITTEST_PROPNEXPORTER_MOCK_H

#include "filemgmt_libn.h"
#include "prop_n_exporter.h"

#include <cstdio>
#include <filesystem>
#include <gmock/gmock.h>
#include <tuple>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;
using std::tuple;

class IPropNExporterMock {
public:
    virtual ~IPropNExporterMock() = default;
    virtual bool InitArgs(std::function<bool()> argcChecker) = 0;
    virtual napi_value GetThisVar() = 0;
    virtual NVal CreateUndefined(napi_env env) = 0;
    virtual napi_value GetArg(size_t argPos) = 0;
    virtual tuple<bool, int32_t> ToInt32() = 0;
    virtual void ThrowErr(napi_env env, std::string errMsg) = 0;
};

class PropNExporterMock : public IPropNExporterMock {
public:
MOCK_METHOD(bool, InitArgs, (std::function<bool()>), (override));
MOCK_METHOD(napi_value, GetThisVar, (), (override));
MOCK_METHOD(NVal, CreateUndefined, (napi_env), (override));
MOCK_METHOD(napi_value, GetArg, (size_t), (override));
MOCK_METHOD((tuple<bool, int32_t>), ToInt32, (), (override));
MOCK_METHOD(void, ThrowErr, (napi_env, std::string), (override));

public:
    static std::shared_ptr<PropNExporterMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<PropNExporterMock> pPropNExporterMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_PROPNEXPORTER_MOCK_H