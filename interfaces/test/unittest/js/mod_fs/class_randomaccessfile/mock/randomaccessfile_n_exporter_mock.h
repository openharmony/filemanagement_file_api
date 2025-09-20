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
 
#ifndef INTERFACES_TEST_UNITTEST_RANDOMACCESSFILENEXPORTER_MOCK_H
#define INTERFACES_TEST_UNITTEST_RANDOMACCESSFILENEXPORTER_MOCK_H

#include "filemgmt_libn.h"
#include "randomaccessfile_n_exporter.h"
#include "randomaccessfile_entity.h"

#include <cstdio>
#include <filesystem>
#include <gmock/gmock.h>
#include <tuple>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;

class IRandomAccessFileNExporterMock {
public:
    virtual ~IRandomAccessFileNExporterMock() = default;
    virtual bool InitArgs(std::function<bool()> argcChecker) = 0;
    virtual napi_value GetThisVar() = 0;
    virtual napi_status napi_unwrap(napi_env env, napi_value js_object, void **result) = 0;
    virtual napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result) = 0;
    virtual RandomAccessFileEntity* RemoveEntityOfFinal(napi_env env, napi_value napi_value) = 0;
    virtual NVal CreateUndefined(napi_env env) = 0;
};

class RandomAccessFileNExporterMock : public IRandomAccessFileNExporterMock {
public:
MOCK_METHOD(bool, InitArgs, (std::function<bool()>), (override));
MOCK_METHOD(napi_value, GetThisVar, (), (override));
MOCK_METHOD(napi_status, napi_unwrap, (napi_env, napi_value, void **), (override));
MOCK_METHOD(napi_status, napi_remove_wrap, (napi_env, napi_value, void **), (override));
MOCK_METHOD(RandomAccessFileEntity*, RemoveEntityOfFinal, (napi_env, napi_value), (override));
MOCK_METHOD(NVal, CreateUndefined, (napi_env), (override));

public:
    static std::shared_ptr<RandomAccessFileNExporterMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static std::shared_ptr<RandomAccessFileNExporterMock> randomAccessFileNExporterMock;
    static bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_RANDOMACCESSFILENEXPORTER_MOCK_H