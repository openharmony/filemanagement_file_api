/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "napi_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace LibN {
namespace Test {

thread_local std::shared_ptr<NapiMock> NapiMock::napiMock = nullptr;
thread_local bool NapiMock::mockable = false;

std::shared_ptr<NapiMock> NapiMock::GetMock()
{
    if (napiMock == nullptr) {
        napiMock = std::make_shared<NapiMock>();
    }
    return napiMock;
}

void NapiMock::EnableMock()
{
    mockable = true;
}

void NapiMock::DisableMock()
{
    napiMock = nullptr;
    mockable = false;
}

bool NapiMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::LibN::Test;

napi_status napi_define_class(napi_env env, const char *name, size_t length, napi_callback constructor, void *data,
    size_t property_count, const napi_property_descriptor *properties, napi_value *result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_define_class(
            env, name, length, constructor, data, property_count, properties, result);
    }

    static napi_status (*realNapiDefineClass)(napi_env, const char *, size_t, napi_callback, void *, size_t,
        const napi_property_descriptor *, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, const char *, size_t, napi_callback, void *, size_t,
            const napi_property_descriptor *, napi_value *))dlsym(RTLD_NEXT, "napi_define_class");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_define_class: " << dlerror();
        }
        return func;
    }();

    if (!realNapiDefineClass) {
        return napi_ok;
    }

    return realNapiDefineClass(env, name, length, constructor, data, property_count, properties, result);
}

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref *result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_create_reference(env, value, initial_refcount, result);
    }

    static napi_status (*realNapiCreateReference)(napi_env, napi_value, uint32_t, napi_ref *) = []() {
        auto func =
            (napi_status(*)(napi_env, napi_value, uint32_t, napi_ref *))dlsym(RTLD_NEXT, "napi_create_reference");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_create_reference: " << dlerror();
        }
        return func;
    }();

    if (!realNapiCreateReference) {
        return napi_ok;
    }

    return realNapiCreateReference(env, value, initial_refcount, result);
}

napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void *arg), void *arg)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_add_env_cleanup_hook(env, fun, arg);
    }

    static napi_status (*realNapiAddEnvCleanupHook)(napi_env, void (*)(void *), void *) = []() {
        auto func = (napi_status(*)(napi_env, void (*)(void *), void *))dlsym(RTLD_NEXT, "napi_add_env_cleanup_hook");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_add_env_cleanup_hook: " << dlerror();
        }
        return func;
    }();

    if (!realNapiAddEnvCleanupHook) {
        return napi_ok;
    }

    return realNapiAddEnvCleanupHook(env, fun, arg);
}

napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_delete_reference(env, ref);
    }

    static napi_status (*realNapiDeleteReference)(napi_env, napi_ref) = []() {
        auto func = (napi_status(*)(napi_env, napi_ref))dlsym(RTLD_NEXT, "napi_delete_reference");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_delete_reference: " << dlerror();
        }
        return func;
    }();

    if (!realNapiDeleteReference) {
        return napi_ok;
    }

    return realNapiDeleteReference(env, ref);
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_get_reference_value(env, ref, result);
    }

    static napi_status (*realNapiGetReferenceValue)(napi_env, napi_ref, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, napi_ref, napi_value *))dlsym(RTLD_NEXT, "napi_get_reference_value");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_get_reference_value: " << dlerror();
        }
        return func;
    }();

    if (!realNapiGetReferenceValue) {
        return napi_ok;
    }

    return realNapiGetReferenceValue(env, ref, result);
}

napi_status napi_new_instance(
    napi_env env, napi_value constructor, size_t argc, const napi_value *argv, napi_value *result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_new_instance(env, constructor, argc, argv, result);
    }

    static napi_status (*realNapiNewInstance)(napi_env, napi_value, size_t, const napi_value *, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, size_t, const napi_value *, napi_value *))dlsym(
            RTLD_NEXT, "napi_new_instance");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_new_instance: " << dlerror();
        }
        return func;
    }();

    if (!realNapiNewInstance) {
        return napi_ok;
    }

    return realNapiNewInstance(env, constructor, argc, argv, result);
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void **result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_unwrap(env, js_object, result);
    }

    static napi_status (*realNapiUnwrap)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_unwrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_unwrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapiUnwrap) {
        return napi_ok;
    }

    return realNapiUnwrap(env, js_object, result);
}

napi_status napi_wrap(napi_env env, napi_value js_object, void *native_object, napi_finalize finalize_cb,
    void *finalize_hint, napi_ref *result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_wrap(env, js_object, native_object, finalize_cb, finalize_hint, result);
    }

    static napi_status (*realNapiWrap)(napi_env, napi_value, void *, napi_finalize, void *, napi_ref *) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void *, napi_finalize, void *, napi_ref *))dlsym(
            RTLD_NEXT, "napi_wrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_wrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapiWrap) {
        return napi_ok;
    }

    return realNapiWrap(env, js_object, native_object, finalize_cb, finalize_hint, result);
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result)
{
    if (NapiMock::IsMockable()) {
        return NapiMock::GetMock()->napi_remove_wrap(env, js_object, result);
    }

    static napi_status (*realNapiRemoveWrap)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_remove_wrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_remove_wrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapiRemoveWrap) {
        return napi_ok;
    }

    return realNapiRemoveWrap(env, js_object, result);
}

} // extern "C"
#endif