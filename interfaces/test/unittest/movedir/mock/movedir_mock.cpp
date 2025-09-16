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

#include "movedir_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

thread_local std::shared_ptr<MovedirMock> MovedirMock::movedirMock = nullptr;
thread_local bool MovedirMock::mockable = false;

std::shared_ptr<MovedirMock> MovedirMock::GetMock()
{
    if (movedirMock == nullptr) {
        movedirMock = std::make_shared<MovedirMock>();
    }
    return movedirMock;
}

void MovedirMock::EnableMock()
{
    mockable = true;
}

void MovedirMock::DisableMock()
{
    movedirMock = nullptr;
    mockable = false;
}

bool MovedirMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
using namespace OHOS::FileManagement::ModuleFileIO::Test;

extern "C" {
int stat(const char *path, struct stat *buf)
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->stat(path, buf);
    }

    static int (*realStat)(const char *, struct stat *) = []() {
        auto func = (int (*)(const char *, struct stat *))dlsym(RTLD_NEXT, "stat");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real stat: " << dlerror();
        }
        return func;
    }();

    if (!realStat) {
        return 0;
    }

    return realStat(path, buf);
}
}

napi_status napi_create_array(napi_env env, napi_value *result)
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->napi_create_array(env, result);
    }

    static napi_status (*realNapiCreateArray)(napi_env, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, napi_value *))dlsym(RTLD_NEXT, "napi_create_array");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_create_array: " << dlerror();
        }
        return func;
    }();

    if (!realNapiCreateArray) {
        return napi_ok;
    }

    return realNapiCreateArray(env, result);
}

bool NFuncArg::InitArgs(std::function<bool()> argcChecker)
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->InitArgs(argcChecker);
    }

    static bool (*realInitArgs)(std::function<bool()>) = []() {
        auto func = (bool (*)(std::function<bool()>))dlsym(RTLD_NEXT, "InitArgs");
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

void NError::ThrowErr(napi_env env)
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->ThrowErr(env);
    }

    static void (*realThrowErr)(napi_env) = []() {
        auto func = (void (*)(napi_env))dlsym(RTLD_NEXT, "ThrowErr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErr: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env);
}

void NError::ThrowErrAddData(napi_env env, int errCode, napi_value data)
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->ThrowErrAddData(env, errCode, data);
    }

    static void (*realThrowErrAddData)(napi_env, int, napi_value) = []() {
        auto func = (void (*)(napi_env, int, napi_value))dlsym(RTLD_NEXT, "ThrowErrAddData");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErrAddData: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErrAddData) {
        return;
    }

    return realThrowErrAddData(env, errCode, data);
}

std::tuple<bool, std::unique_ptr<char[]>, size_t> NVal::ToUTF8StringPath() const
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->ToUTF8StringPath();
    }

    static std::tuple<bool, std::unique_ptr<char[]>, size_t> (*realToUTF8StringPath)() = []() {
        auto func = (std::tuple<bool, std::unique_ptr<char[]>, size_t>(*)())dlsym(RTLD_NEXT, "ToUTF8StringPath");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToUTF8StringPath: " << dlerror();
        }
        return func;
    }();

    if (!realToUTF8StringPath) {
        return { false, nullptr, -1 };
    }

    return realToUTF8StringPath();
}

std::tuple<bool, int32_t> NVal::ToInt32(int32_t defaultValue) const
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->ToInt32(defaultValue);
    }

    static std::tuple<bool, int32_t> (*realToInt32)(int32_t) = []() {
        auto func = (std::tuple<bool, int32_t>(*)(int32_t))dlsym(RTLD_NEXT, "ToInt32");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToInt32: " << dlerror();
        }
        return func;
    }();

    if (!realToInt32) {
        return { false, -1 };
    }

    return realToInt32(defaultValue);
}

size_t NFuncArg::GetArgc(void) const
{
    if (MovedirMock::IsMockable()) {
        return MovedirMock::GetMock()->GetArgc();
    }

    static size_t (*realGetArgc)() = []() {
        auto func = (size_t(*)())dlsym(RTLD_NEXT, "GetArgc");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetArgc: " << dlerror();
        }
        return func;
    }();

    if (!realGetArgc) {
        return -1;
    }

    return realGetArgc();
}
#endif