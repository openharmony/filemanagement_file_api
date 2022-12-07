/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "n_val.h"

#include <string>

#include "../log.h"
#include "../uni_error.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;

NVal::NVal(napi_env nEnv, napi_value nVal = nullptr) : env_(nEnv), val_(nVal) {}

NVal::operator bool() const
{
    return env_ && val_;
}

bool NVal::TypeIs(napi_valuetype expType) const
{
    if (!*this) {
        return false;
    }

    napi_valuetype valueType;
    napi_typeof(env_, val_, &valueType);
    if (expType != valueType) {
        return false;
    }

    return true;
}

bool NVal::TypeIsError(bool checkErrno) const
{
    if (!*this) {
        return false;
    }

    bool res = false;
    napi_is_error(env_, val_, &res);

    return res;
}

tuple<bool, unique_ptr<char[]>, size_t> NVal::ToUTF8String() const
{
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf8(env_, val_, nullptr, -1, &strLen);
    if (status != napi_ok) {
        return { false, nullptr, 0 };
    }

    size_t bufLen = strLen + 1;
    unique_ptr<char[]> str = make_unique<char[]>(bufLen);
    status = napi_get_value_string_utf8(env_, val_, str.get(), bufLen, &strLen);
    return make_tuple(status == napi_ok, move(str), strLen);
}

tuple<bool, unique_ptr<char[]>, size_t> NVal::ToUTF16String() const
{
#ifdef FILE_SUBSYSTEM_DEBUG_LOCAL
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf16(env_, val_, nullptr, -1, &strLen);
    if (status != napi_ok) {
        return { false, nullptr, 0 };
    }

    auto str = make_unique<char16_t[]>(++strLen);
    status = napi_get_value_string_utf16(env_, val_, str.get(), strLen, nullptr);
    if (status != napi_ok) {
        return { false, nullptr, 0 };
    }

    strLen = reinterpret_cast<char *>(str.get() + strLen) - reinterpret_cast<char *>(str.get());
    auto strRet = unique_ptr<char[]>(reinterpret_cast<char *>(str.release()));
    return { true, move(strRet), strLen };
#else
    // Note that quickjs doesn't support utf16
    return ToUTF8String();
#endif
}

tuple<bool, void *> NVal::ToPointer() const
{
    void *res = nullptr;
    napi_status status = napi_get_value_external(env_, val_, &res);
    return make_tuple(status == napi_ok, res);
}

tuple<bool, bool> NVal::ToBool() const
{
    bool flag = false;
    napi_status status = napi_get_value_bool(env_, val_, &flag);
    return make_tuple(status == napi_ok, flag);
}

tuple<bool, int32_t> NVal::ToInt32() const
{
    int32_t res = 0;
    napi_status status = napi_get_value_int32(env_, val_, &res);
    return make_tuple(status == napi_ok, res);
}

tuple<bool, int64_t> NVal::ToInt64() const
{
    int64_t res = 0;
    napi_status status = napi_get_value_int64(env_, val_, &res);
    return make_tuple(status == napi_ok, res);
}

tuple<bool, double> NVal::ToDouble() const
{
    double res = 0;
    napi_status status = napi_get_value_double(env_, val_, &res);
    return make_tuple(status == napi_ok, res);
}

tuple<bool, uint64_t, bool> NVal::ToUint64() const
{
    uint64_t res = 0;
    bool lossless = false;
    napi_status status = napi_get_value_bigint_uint64(env_, val_, &res, &lossless);
    return make_tuple(status == napi_ok, res, lossless);
}

tuple<bool, vector<string>, uint32_t> NVal::ToStringArray()
{
    napi_status status;
    uint32_t size;
    status = napi_get_array_length(env_, val_, &size);
    vector<string> stringArray;
    napi_value result;
    for (uint32_t i = 0; i < size; i++) {
        status = napi_get_element(env_, val_, i, &result);
        auto [succ, str, ignore] = NVal(env_, result).ToUTF8String();
        stringArray.push_back(string(str.get()));
    }
    return make_tuple(status == napi_ok, stringArray, size);
}

tuple<bool, void *, size_t> NVal::ToArraybuffer() const
{
    void *buf = nullptr;
    size_t bufLen = 0;
    bool status = napi_get_arraybuffer_info(env_, val_, &buf, &bufLen);
    return make_tuple(status == napi_ok, buf, bufLen);
}

tuple<bool, void *, size_t> NVal::ToTypedArray() const
{
    napi_typedarray_type type;
    napi_value in_array_buffer = nullptr;
    size_t byte_offset;
    size_t length;
    void *data = nullptr;
    napi_status status =
        napi_get_typedarray_info(env_, val_, &type, &length, (void **)&data, &in_array_buffer, &byte_offset);
    return make_tuple(status == napi_ok, data, length);
}

bool NVal::HasProp(string propName) const
{
    bool res = false;

    if (!env_ || !val_ || !TypeIs(napi_object))
        return false;
    napi_status status = napi_has_named_property(env_, val_, propName.c_str(), &res);
    return (status == napi_ok) && res;
}

NVal NVal::GetProp(string propName) const
{
    if (!HasProp(propName)) {
        return { env_, nullptr };
    }

    napi_value prop = nullptr;
    napi_status status = napi_get_named_property(env_, val_, propName.c_str(), &prop);
    if (status != napi_ok) {
        return { env_, nullptr };
    }

    return NVal(env_, prop);
}

bool NVal::AddProp(vector<napi_property_descriptor> &&propVec) const
{
    if (!TypeIs(napi_valuetype::napi_object)) {
        HILOGE("INNER BUG. Prop should only be added to objects");
        return false;
    }

    napi_status status = napi_define_properties(env_, val_, propVec.size(), propVec.data());
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot define properties because of %{public}d", status);
        return false;
    }

    return true;
}

bool NVal::AddProp(string propName, napi_value val) const
{
    if (!TypeIs(napi_valuetype::napi_object) || HasProp(propName)) {
        HILOGE("INNER BUG. Prop should only be added to objects");
        return false;
    }

    napi_status status = napi_set_named_property(env_, val_, propName.c_str(), val);
    if (status != napi_ok) {
        HILOGE("INNER BUG. Cannot set named property because of %{public}d", status);
        return false;
    }

    return true;
}

NVal NVal::CreateUndefined(napi_env env)
{
    napi_value res = nullptr;
    napi_get_undefined(env, &res);
    return { env, res };
}

NVal NVal::CreateBigInt64(napi_env env, int64_t val)
{
    napi_value res = nullptr;
    napi_create_bigint_int64(env, val, &res);
    return { env, res };
}

NVal NVal::CreateInt64(napi_env env, int64_t val)
{
    napi_value res = nullptr;
    napi_create_int64(env, val, &res);
    return { env, res };
}

NVal NVal::CreateInt32(napi_env env, int32_t val)
{
    napi_value res = nullptr;
    napi_create_int32(env, val, &res);
    return { env, res };
}

NVal NVal::CreateObject(napi_env env)
{
    napi_value res = nullptr;
    napi_create_object(env, &res);
    return { env, res };
}

NVal NVal::CreateBool(napi_env env, bool val)
{
    napi_value res = nullptr;
    napi_get_boolean(env, val, &res);
    return { env, res };
}

NVal NVal::CreateUTF8String(napi_env env, std::string str)
{
    napi_value res = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &res);
    return { env, res };
}

NVal NVal::CreateUTF8String(napi_env env, const char* str, ssize_t len)
{
    napi_value res = nullptr;
    napi_create_string_utf8(env, str, len, &res);
    return { env, res };
}

NVal NVal::CreateUint8Array(napi_env env, void *buf, size_t bufLen)
{
    napi_value output_buffer = nullptr;
    napi_create_external_arraybuffer(
        env,
        buf,
        bufLen,
        [](napi_env env, void *finalize_data, void *finalize_hint) { free(finalize_data); },
        NULL,
        &output_buffer);
    napi_value output_array = nullptr;
    napi_create_typedarray(env, napi_uint8_array, bufLen, output_buffer, 0, &output_array);
    return { env, output_array };
}

NVal NVal::CreateArrayString(napi_env env, vector<string> strs)
{
    napi_value res = nullptr;
    napi_create_array(env, &res);
    for (size_t i = 0; i < strs.size(); i++) {
        napi_value filename;
        napi_create_string_utf8(env, strs[i].c_str(), strs[i].length(), &filename);
        napi_set_element(env, res, i, filename);
    }
    return {env, res};
}

tuple<NVal, void *> NVal::CreateArrayBuffer(napi_env env, size_t len)
{
    napi_value val;
    void *buf = nullptr;
    napi_create_arraybuffer(env, len, &buf, &val);
    return { { env, val }, { buf } };
}

napi_property_descriptor NVal::DeclareNapiProperty(const char *name, napi_value val)
{
    return { (name), nullptr, nullptr, nullptr, nullptr, val, napi_default, nullptr };
}

napi_property_descriptor NVal::DeclareNapiStaticProperty(const char *name, napi_value val)
{
    return { (name), nullptr, nullptr, nullptr, nullptr, val, napi_static, nullptr };
}

napi_property_descriptor NVal::DeclareNapiFunction(const char *name, napi_callback func)
{
    return { (name), nullptr, (func), nullptr, nullptr, nullptr, napi_default, nullptr };
}

napi_property_descriptor NVal::DeclareNapiStaticFunction(const char *name, napi_callback func)
{
    return { (name), nullptr, (func), nullptr, nullptr, nullptr, napi_static, nullptr };
}

napi_property_descriptor NVal::DeclareNapiGetter(const char *name, napi_callback getter)
{
    return { (name), nullptr, nullptr, (getter), nullptr, nullptr, napi_default, nullptr };
}

napi_property_descriptor NVal::DeclareNapiSetter(const char *name, napi_callback setter)
{
    return { (name), nullptr, nullptr, nullptr, (setter), nullptr, napi_default, nullptr };
}

napi_property_descriptor NVal::DeclareNapiGetterSetter(const char *name, napi_callback getter, napi_callback setter)
{
    return { (name), nullptr, nullptr, (getter), (setter), nullptr, napi_default, nullptr };
}
} // namespace DistributedFS
} // namespace OHOS