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

#include <mutex>

namespace OHOS {
namespace FileManagement {
namespace LibN {
namespace Test {

namespace {
std::mutex g_mockMutex;
NapiMock *g_currentMock = nullptr;
NapiMock g_defaultMock;
} // namespace

NapiMock &GetNapiMock()
{
    std::lock_guard<std::mutex> lock(g_mockMutex);
    return g_currentMock ? *g_currentMock : g_defaultMock;
}

void SetNapiMock(NapiMock *mock)
{
    std::lock_guard<std::mutex> lock(g_mockMutex);
    g_currentMock = mock;
}

void ResetNapiMock()
{
    SetNapiMock(nullptr);
}

} // namespace Test
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#ifdef ENABLE_NAPI_MOCK
extern "C" {
using namespace OHOS::FileManagement::LibN::Test;

napi_status napi_define_class(napi_env env, const char *name, size_t length, napi_callback constructor, void *data,
    size_t property_count, const napi_property_descriptor *properties, napi_value *result)
{
    return GetNapiMock().napi_define_class(env, name, length, constructor, data, property_count, properties, result);
}

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref *result)
{
    return GetNapiMock().napi_create_reference(env, value, initial_refcount, result);
}

napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void *arg), void *arg)
{
    return GetNapiMock().napi_add_env_cleanup_hook(env, fun, arg);
}

napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    return GetNapiMock().napi_delete_reference(env, ref);
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result)
{
    return GetNapiMock().napi_get_reference_value(env, ref, result);
}

napi_status napi_new_instance(
    napi_env env, napi_value constructor, size_t argc, const napi_value *argv, napi_value *result)
{
    return GetNapiMock().napi_new_instance(env, constructor, argc, argv, result);
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void **result)
{
    return GetNapiMock().napi_unwrap(env, js_object, result);
}

napi_status napi_wrap(napi_env env, napi_value js_object, void *native_object, napi_finalize finalize_cb,
    void *finalize_hint, napi_ref *result)
{
    return GetNapiMock().napi_wrap(env, js_object, native_object, finalize_cb, finalize_hint, result);
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result)
{
    return GetNapiMock().napi_remove_wrap(env, js_object, result);
}

} // extern "C"
#endif