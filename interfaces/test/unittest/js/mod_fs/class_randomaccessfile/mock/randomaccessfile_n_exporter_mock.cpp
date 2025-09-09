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

#include "randomaccessfile_n_exporter_mock.h"
#include "n_class.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

std::shared_ptr<RandomAccessFileNExporterMock>
    RandomAccessFileNExporterMock::randomAccessFileNExporterMock = nullptr;
bool RandomAccessFileNExporterMock::mockable = false;

std::shared_ptr<RandomAccessFileNExporterMock> RandomAccessFileNExporterMock::GetMock()
{
    if (randomAccessFileNExporterMock == nullptr) {
        randomAccessFileNExporterMock = std::make_shared<RandomAccessFileNExporterMock>();
    }
    return randomAccessFileNExporterMock;
}

void RandomAccessFileNExporterMock::EnableMock()
{
    mockable = true;
}

void RandomAccessFileNExporterMock::DisableMock()
{
    randomAccessFileNExporterMock = nullptr;
    mockable = false;
}

bool RandomAccessFileNExporterMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

using namespace OHOS::FileManagement::ModuleFileIO::Test;

bool NFuncArg::InitArgs(std::function<bool()> argcChecker)
{
    if (RandomAccessFileNExporterMock::IsMockable()) {
        return RandomAccessFileNExporterMock::GetMock()->InitArgs(argcChecker);
    }

    static bool (*realInitArgs)(std::function<bool()>) = []() {
        auto func = (bool(*)(std::function<bool()>))dlsym(RTLD_NEXT, "InitArgs");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real InitArgs: " << dlerror();
        }
        return func;
    }();

    if (!realInitArgs) {
        return false;
    }

    return realInitArgs(argcChecker);
}

napi_value NFuncArg::GetThisVar() const
{
    if (RandomAccessFileNExporterMock::IsMockable()) {
        return RandomAccessFileNExporterMock::GetMock()->GetThisVar();
    }

    static napi_value (*realGetThisVar)() = []() {
        auto func = (napi_value(*)())dlsym(RTLD_NEXT, "GetThisVar");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetThisVar: " << dlerror();
        }
        return func;
    }();

    if (!realGetThisVar) {
        return nullptr;
    }

    return realGetThisVar();
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void **result)
{
    if (RandomAccessFileNExporterMock::IsMockable()) {
        return RandomAccessFileNExporterMock::GetMock()->napi_unwrap(env, js_object, result);
    }

    static napi_status (*realNapi)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_unwrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_unwrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, js_object, result);
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result)
{
    if (RandomAccessFileNExporterMock::IsMockable()) {
        return RandomAccessFileNExporterMock::GetMock()->napi_remove_wrap(env, js_object, result);
    }

    static napi_status (*realNapi)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_remove_wrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_remove_wrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, js_object, result);
}

NVal NVal::CreateUndefined(napi_env env)
{
    if (RandomAccessFileNExporterMock::IsMockable()) {
        return RandomAccessFileNExporterMock::GetMock()->CreateUndefined(env);
    }

    static NVal (*realCreateUndefined)(napi_env) = []() {
        auto func = (NVal(*)(napi_env))dlsym(RTLD_NEXT, "CreateUndefined");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real CreateUndefined: " << dlerror();
        }
        return func;
    }();

    if (!realCreateUndefined) {
        return {nullptr, nullptr};
    }

    return realCreateUndefined(env);
}