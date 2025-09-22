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

#ifndef INTERFACES_TEST_UNITTEST_FILEMGMT_LIBN_TEST_INCLUDE_NAPI_MOCK_H
#define INTERFACES_TEST_UNITTEST_FILEMGMT_LIBN_TEST_INCLUDE_NAPI_MOCK_H

#pragma once
#include <gmock/gmock.h>
#include "n_napi.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
namespace Test {

class INapiMock {
public:
    virtual ~INapiMock() = default;

    virtual napi_status napi_define_class(napi_env env, const char *name, size_t length, napi_callback constructor,
        void *data, size_t property_count, const napi_property_descriptor *properties, napi_value *result) = 0;

    virtual napi_status napi_create_reference(
        napi_env env, napi_value value, uint32_t initial_refcount, napi_ref *result) = 0;

    virtual napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void *arg), void *arg) = 0;

    virtual napi_status napi_delete_reference(napi_env env, napi_ref ref) = 0;

    virtual napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value *result) = 0;

    virtual napi_status napi_new_instance(
        napi_env env, napi_value constructor, size_t argc, const napi_value *argv, napi_value *result) = 0;

    virtual napi_status napi_unwrap(napi_env env, napi_value js_object, void **result) = 0;

    virtual napi_status napi_wrap(napi_env env, napi_value js_object, void *native_object, napi_finalize finalize_cb,
        void *finalize_hint, napi_ref *result) = 0;

    virtual napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result) = 0;
};

class NapiMock : public INapiMock {
public:
    MOCK_METHOD(napi_status, napi_define_class,
        (napi_env, const char *, size_t, napi_callback, void *, size_t, const napi_property_descriptor *, napi_value *),
        (override));

    MOCK_METHOD(napi_status, napi_create_reference, (napi_env, napi_value, uint32_t, napi_ref *), (override));

    MOCK_METHOD(napi_status, napi_add_env_cleanup_hook, (napi_env env, void (*fun)(void *arg), void *arg), (override));

    MOCK_METHOD(napi_status, napi_delete_reference, (napi_env env, napi_ref ref), (override));

    MOCK_METHOD(napi_status, napi_get_reference_value, (napi_env env, napi_ref ref, napi_value *result), (override));

    MOCK_METHOD(napi_status, napi_new_instance,
        (napi_env env, napi_value constructor, size_t argc, const napi_value *argv, napi_value *result), (override));

    MOCK_METHOD(napi_status, napi_unwrap, (napi_env env, napi_value js_object, void **result), (override));

    MOCK_METHOD(napi_status, napi_wrap,
        (napi_env env, napi_value js_object, void *native_object, napi_finalize finalize_cb, void *finalize_hint,
            napi_ref *result), (override));

    MOCK_METHOD(napi_status, napi_remove_wrap, (napi_env env, napi_value js_object, void **result), (override));
};

NapiMock &GetNapiMock();
void SetNapiMock(NapiMock *mock);
void ResetNapiMock();

class ScopedNapiMock {
public:
    explicit ScopedNapiMock(NapiMock *mock) : mock_(mock)
    {
        SetNapiMock(mock_);
    }

    ~ScopedNapiMock()
    {
        ResetNapiMock();
    }

    NapiMock &GetMock()
    {
        return *mock_;
    }

    ScopedNapiMock(const ScopedNapiMock &) = delete;
    ScopedNapiMock &operator=(const ScopedNapiMock &) = delete;

private:
    NapiMock *mock_;
};

} // namespace Test
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_TEST_UNITTEST_FILEMGMT_LIBN_TEST_INCLUDE_NAPI_MOCK_H