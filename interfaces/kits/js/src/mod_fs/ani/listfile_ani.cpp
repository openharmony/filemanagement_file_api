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

#include "filemgmt_libhilog.h"
#include "listfile_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIo {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

tuple<bool, bool> ParseBooleanParam(ani_env *env, ani_object obj, string tag)
{
    ani_ref bool_ref;
    ani_boolean isUndefined;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &bool_ref)) {
        return { false, false };
    }
    env->Reference_IsUndefined(bool_ref, &isUndefined);
    if (!isUndefined) {
        return { true, false };
    }
    ani_boolean bool_ref_res;
    if (ANI_OK !=
        env->Object_CallMethodByName_Boolean(static_cast<ani_object>(bool_ref), "unboxed", ":Z", &bool_ref_res)) {
        return { false, false };
    }
    return { true, static_cast<bool>(bool_ref_res) };
}

tuple<bool, int> ParseIntParam(ani_env *env, ani_object obj, string tag)
{
    int result = 0;
    ani_boolean isUndefined;
    ani_ref result_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &result_ref)) {
        return { false, result };
    }
    env->Reference_IsUndefined(result_ref, &isUndefined);
    if (isUndefined) {
        return { false, result };
    }
    ani_int result_ref_res;
    if (ANI_OK !=
        env->Object_CallMethodByName_Int(static_cast<ani_object>(result_ref), "intValue", nullptr, &result_ref_res)) {
        result = -1;
        return { false, result };
    }
    result = static_cast<int>(result_ref_res);
    return { true, result };
}

tuple<bool, optional<double>> ParseDoubleParam(ani_env *env, ani_object obj, string tag)
{
    ani_boolean isUndefined;
    ani_ref result_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &result_ref)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(result_ref, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    ani_double result_ref_res;
    if (ANI_OK != env->Object_CallMethodByName_Double(
                      static_cast<ani_object>(result_ref), "doubleValue", nullptr, &result_ref_res)) {
        return { false, nullopt };
    }
    double result = static_cast<double>(result_ref_res);
    return { true, make_optional<double>(result) };
}

tuple<bool, optional<vector<string>>> AnalyzerArrayString(ani_env *env, ani_object obj, string tag)
{
    ani_boolean isUndefined;
    ani_ref result_ref;
    vector<string> strings;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, tag.c_str(), &result_ref)) {
        return { false, nullopt };
    }
    env->Reference_IsUndefined(result_ref, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(static_cast<ani_object>(result_ref), "length", &length) ||
        length == 0) {
        return { false, nullopt };
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref stringEntryRef;
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(result_ref), "$_get",
                          "I:Lstd/core/Object;", &stringEntryRef, (ani_int)i)) {
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

tuple<bool, optional<FsFileFilter>> AnalyzeFilter(ani_env *env, ani_object obj)
{
    FsFileFilter filter;

    auto [succfileSizeOver, fileSizeOver] = ParseIntParam(env, obj, "fileSizeOver");
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
    filter.SetFileSizeOver(lastModifiedAfter);

    auto [succSuffix, suffix] = AnalyzerArrayString(env, obj, "suffix");
    if (!succSuffix) {
        HILOGE("Illegal option.suffix parameter");
        return { false, move(filter) };
    }
    filter.SetSuffix(move(suffix));

    auto [succDisplayName, displayName] = AnalyzerArrayString(env, obj, "displayName");
    if (!succDisplayName) {
        HILOGE("Illegal option.displayName parameter");
        return { false, move(filter) };
    }
    filter.SetDisplayName(move(displayName));

    return { true, move(filter) };
}

tuple<bool, optional<FsListFileOptions>> AnalyzeArgs(ani_env *env, ani_object obj)
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

    auto [succlistNum, listNumRes] = ParseIntParam(env, obj, "listNum");
    if (!succlistNum) {
        HILOGE("Invalid listNum");
        return { false, nullopt };
    }
    result.listNum = (int)listNumRes;

    ani_ref filter_ref;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, "filter", &filter_ref)) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    env->Reference_IsUndefined(filter_ref, &isUndefined);
    if (isUndefined) {
        return { true, make_optional<FsListFileOptions>(result) };
    }
    auto [succFilter, filterFilterClass] = AnalyzeFilter(env, static_cast<ani_object>(filter_ref));
    if (!succFilter) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    result.filter = move(filterFilterClass);

    return { true, make_optional<FsListFileOptions>(result) };
}

ani_array_ref ListFileAni::ListFileSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object obj)
{
    auto [succPath, srcPath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        return nullptr;
    }

    auto [succOpt, opt] = AnalyzeArgs(env, obj);
    if (!succOpt) {
        HILOGE("Invalid options Arguments");
        return nullptr;
    }

    auto ret = ListFileCore::DoListFile(srcPath, opt);
    if (!ret.IsSuccess()) {
        HILOGE("DoListFile failed");
        return nullptr;
    }

    auto fileList = ret.GetData().value();
    const std::string *strArray = fileList.data();
    auto [succ, result] = TypeConverter::ToAniStringList(env, strArray, fileList.size());
    if (!succ) {
        HILOGE("list file result value to ani_string list failed");
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIo
} // namespace FileManagement
} // namespace OHOS