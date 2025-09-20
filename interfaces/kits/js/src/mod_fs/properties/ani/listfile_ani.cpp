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

#include "listfile_ani.h"

#include "ani_signature.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "listfile_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

tuple<bool, bool> ParseBooleanParam(ani_env *env, ani_object obj, string tag)
{
    ani_ref boolRef;
    ani_boolean isUndefined;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &boolRef)) {
        return { false, false };
    }
    env->Reference_IsUndefined(boolRef, &isUndefined);
    if (isUndefined) {
        return { true, false };
    }
    auto unboxedDesc = BoxedTypes::Boolean::unboxedDesc.c_str();
    auto unboxedSig = BoxedTypes::Boolean::unboxedSig.c_str();
    ani_boolean boolRef_res;
    if (ANI_OK != env->Object_CallMethodByName_Boolean(
        static_cast<ani_object>(boolRef), unboxedDesc, unboxedSig, &boolRef_res)) {
        return { false, false };
    }
    return { true, static_cast<bool>(boolRef_res) };
}

tuple<bool, long> ParseLongParam(ani_env *env, ani_object obj, string tag)
{
    long result = 0;
    ani_boolean isUndefined;
    ani_ref result_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &result_ref)) {
        return { false, result };
    }

    env->Reference_IsUndefined(result_ref, &isUndefined);
    if (isUndefined) {
        return { true, result };
    }
    
    ani_long result_ref_res;
    if (ANI_OK != env->Object_CallMethodByName_Long(
        static_cast<ani_object>(result_ref), "toLong", nullptr, &result_ref_res)) {
        result = -1;
        return { false, result };
    }
    result = static_cast<int64_t>(result_ref_res);
    return { true, result };
}

tuple<bool, int> ParseIntParam(ani_env *env, ani_object obj, string tag)
{
    int result = 0;
    ani_boolean isUndefined;
    ani_ref resultRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &resultRef)) {
        return { false, result };
    }
    env->Reference_IsUndefined(resultRef, &isUndefined);
    if (isUndefined) {
        return { true, result };
    }
    ani_int resultRefRes;
    if (ANI_OK != env->Object_CallMethodByName_Int(
        static_cast<ani_object>(resultRef), BasicTypesConverter::toInt.c_str(), nullptr, &resultRefRes)) {
        result = -1;
        return { false, result };
    }
    result = static_cast<int>(resultRefRes);
    return { true, result };
}

tuple<bool, optional<double>> ParseDoubleParam(ani_env *env, ani_object obj, string tag)
{
    ani_boolean isUndefined;
    ani_ref resultRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &resultRef)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(resultRef, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    ani_double resultRefRes;
    if (ANI_OK != env->Object_CallMethodByName_Double(
        static_cast<ani_object>(resultRef), BasicTypesConverter::toDouble.c_str(), nullptr, &resultRefRes)) {
        return { false, nullopt };
    }
    double result = static_cast<double>(resultRefRes);
    return { true, make_optional<double>(result) };
}

tuple<bool, optional<vector<string>>> ParseArrayString(ani_env *env, ani_object obj, string tag)
{
    ani_boolean isUndefined;
    ani_ref resultRef;
    vector<string> strings;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &resultRef)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(resultRef, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    ani_int length;
    if (ANI_OK != env->Object_GetPropertyByName_Int(
        static_cast<ani_object>(resultRef), "length", &length) || length == 0) {
        return { false, nullopt };
    }
    auto getterDesc = BuiltInTypes::Array::getterDesc.c_str();
    auto getterSig = BuiltInTypes::Array::objectGetterSig.c_str();
    for (int idx = 0; idx < int(length); idx++) {
        ani_ref stringEntryRef;
        if (ANI_OK != env->Object_CallMethodByName_Ref(
            static_cast<ani_object>(resultRef), getterDesc, getterSig, &stringEntryRef, (ani_int)idx)) {
            return { false, nullopt };
        }
        auto [succ, tmp] = TypeConverter::ToUTF8String(env, static_cast<ani_string>(stringEntryRef));
        if (!succ) {
            return { false, nullopt };
        }
        strings.emplace_back(tmp);
    }
    return { true, make_optional<vector<string>>(move(strings)) };
}

tuple<bool, optional<FsFileFilter>> ParseFilter(ani_env *env, ani_object obj)
{
    FsFileFilter filter;

    auto [succfileSizeOver, fileSizeOver] = ParseLongParam(env, obj, "fileSizeOver");
    if (!succfileSizeOver) {
        HILOGE("Illegal option.fileSizeOver parameter");
        return { false, move(filter) };
    }
    filter.SetFileSizeOver(fileSizeOver);

    auto [succlastModifiedAfter, lastModifiedAfter] = ParseDoubleParam(env, obj, "lastModifiedAfter");
    if (!succlastModifiedAfter) {
        HILOGE("Illegal option.lastModifiedAfter parameter");
        return { false, move(filter) };
    }
    filter.SetLastModifiedAfter(lastModifiedAfter);

    auto [succSuffix, suffix] = ParseArrayString(env, obj, "suffix");
    if (!succSuffix) {
        HILOGE("Illegal option.suffix parameter");
        return { false, move(filter) };
    }
    filter.SetSuffix(move(suffix));

    auto [succDisplayName, displayName] = ParseArrayString(env, obj, "displayName");
    if (!succDisplayName) {
        HILOGE("Illegal option.displayName parameter");
        return { false, move(filter) };
    }
    filter.SetDisplayName(move(displayName));

    return { true, move(filter) };
}

tuple<bool, optional<FsListFileOptions>> ParseArgs(ani_env *env, ani_object obj)
{
    FsListFileOptions result;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succRecursion, recursion] = ParseBooleanParam(env, obj, "recursion");
    if (!succRecursion) {
        HILOGE("Invalid recursion");
        return { false, nullopt };
    }
    result.recursion = recursion;

    auto [succlistNum, listNumRes] = ParseLongParam(env, obj, "listNum");
    if (!succlistNum) {
        HILOGE("Invalid listNum");
        return { false, nullopt };
    }
    result.listNum = listNumRes;

    ani_ref filterRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, "filter", &filterRef)) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    env->Reference_IsUndefined(filterRef, &isUndefined);
    if (isUndefined) {
        return { true, make_optional<FsListFileOptions>(result) };
    }
    auto [succFilter, filterFilterClass] = ParseFilter(env, static_cast<ani_object>(filterRef));
    if (!succFilter) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    result.filter = move(filterFilterClass);

    return { true, make_optional<FsListFileOptions>(result) };
}

ani_array ListFileAni::ListFileSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object obj)
{
    auto [succPath, srcPath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [succOpt, opt] = ParseArgs(env, obj);
    if (!succOpt) {
        HILOGE("Invalid options Arguments");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto ret = ListFileCore::DoListFile(srcPath, opt);
    if (!ret.IsSuccess()) {
        HILOGE("DoListFile failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto fileList = ret.GetData().value();
    const std::string *strArray = fileList.data();
    auto [succ, result] = TypeConverter::ToAniStringList(env, strArray, fileList.size());
    if (!succ) {
        HILOGE("Convert list file result to ani string array failed");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
