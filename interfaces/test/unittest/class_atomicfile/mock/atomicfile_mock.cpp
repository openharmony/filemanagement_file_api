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

#include "atomicfile_mock.h"

#include <dlfcn.h>

using namespace OHOS::FileManagement::ModuleFileIO;

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<AtomicfileMock> AtomicfileMock::atomicfileMock = nullptr;
thread_local bool AtomicfileMock::mockable = false;

std::shared_ptr<AtomicfileMock> AtomicfileMock::GetMock()
{
    if (atomicfileMock == nullptr) {
        atomicfileMock = std::make_shared<AtomicfileMock>();
    }
    return atomicfileMock;
}

void AtomicfileMock::EnableMock()
{
    mockable = true;
}

void AtomicfileMock::DisableMock()
{
    atomicfileMock = nullptr;
    mockable = false;
}

bool AtomicfileMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
using namespace OHOS::FileManagement::ModuleFileIO::Test;

extern "C" {
int rename(const char *old_filename, const char *new_filename)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->rename(old_filename, new_filename);
    }

    static int (*realRename)(const char *, const char *) = []() {
        auto func = (int (*)(const char *, const char *))dlsym(RTLD_NEXT, "rename");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real rename: " << dlerror();
        }
        return func;
    }();

    if (!realRename) {
        return -1;
    }

    return realRename(old_filename, new_filename);
}

int remove(const char *filename)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->remove(filename);
    }

    static int (*realRemove)(const char *) = []() {
        auto func = (int (*)(const char *))dlsym(RTLD_NEXT, "remove");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real remove: " << dlerror();
        }
        return func;
    }();

    if (!realRemove) {
        return -1;
    }

    return realRemove(filename);
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->napi_get_reference_value(env, ref, result);
    }

    static napi_status (*realNapi)(napi_env, napi_ref, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, napi_ref, napi_value *))dlsym(RTLD_NEXT, "napi_get_reference_value");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_get_reference_value: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, ref, result);
}

napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->napi_delete_reference(env, ref);
    }

    static napi_status (*realNapi)(napi_env, napi_ref) = []() {
        auto func = (napi_status(*)(napi_env, napi_ref))dlsym(RTLD_NEXT, "napi_delete_reference");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_delete_reference: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, ref);
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void **result)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->napi_unwrap(env, js_object, result);
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

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype *result)
{
    return AtomicfileMock::GetMock()->napi_typeof(env, value, result);
}
} // extern "C"

bool NFuncArg::InitArgs(std::function<bool()> argcChecker)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->InitArgs(argcChecker);
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

napi_value NFuncArg::GetThisVar() const
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->GetThisVar();
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

void NError::ThrowErr(napi_env env, std::string errMsg)
{
    if (AtomicfileMock::IsMockable()) {
        return AtomicfileMock::GetMock()->ThrowErr(env, errMsg);
    }

    static void (*realThrowErr)(napi_env, std::string) = []() {
        auto func = (void (*)(napi_env, std::string))dlsym(RTLD_NEXT, "ThrowErr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErr: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env, errMsg);
}
#endif