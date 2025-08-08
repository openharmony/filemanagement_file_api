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

#ifndef INTERFACES_TEST_UNITTEST_CLASS_ATOMICFILE_MOCK_ATOMICFILE_MOCK_H
#define INTERFACES_TEST_UNITTEST_CLASS_ATOMICFILE_MOCK_ATOMICFILE_MOCK_H

#include "n_napi.h"
#include "atomicfile_entity.h"
#include "atomicfile_n_exporter.h"
#include "filemgmt_libn.h"

#include <cstdio>
#include <filesystem>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::LibN;

class IAtomicfileMock {
public:
    virtual ~IAtomicfileMock() = default;
    virtual int rename(const char *old_filename, const char *new_filename) = 0;
    virtual int remove(const char *filename) = 0;
    virtual napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) = 0;
    virtual napi_status napi_delete_reference(napi_env env, napi_ref ref) = 0;
    virtual bool InitArgs(std::function<bool()> argcChecker) = 0;
    virtual napi_value GetThisVar() = 0;
    virtual napi_status napi_unwrap(napi_env env, napi_value js_object, void **result) = 0;
    virtual napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype *result) = 0;
    virtual void ThrowErr(napi_env env, std::string errMsg) = 0;
};

class AtomicfileMock : public IAtomicfileMock {
public:
    MOCK_METHOD(int, rename, (const char *, const char *), (override));
    MOCK_METHOD(int, remove, (const char *), (override));
    MOCK_METHOD(napi_status, napi_get_reference_value, (napi_env, napi_ref, napi_value *), (override));
    MOCK_METHOD(napi_status, napi_delete_reference, (napi_env, napi_ref), (override));
    MOCK_METHOD(bool, InitArgs, (std::function<bool()>), (override));
    MOCK_METHOD(napi_value, GetThisVar, (), (override));
    MOCK_METHOD(napi_status, napi_unwrap, (napi_env, napi_value, void **), (override));
    MOCK_METHOD(napi_status, napi_typeof, (napi_env, napi_value, napi_valuetype *), (override));
    MOCK_METHOD(void, ThrowErr, (napi_env, std::string), (override));

public:
    static std::shared_ptr<AtomicfileMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<AtomicfileMock> atomicfileMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_CLASS_ATOMICFILE_MOCK_ATOMICFILE_MOCK_H