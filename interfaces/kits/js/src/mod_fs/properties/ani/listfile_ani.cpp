/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "file_utils.h"
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

static tuple<bool, optional<FsFilter>> ParseFilter(ani_env *env, ani_object obj)
{
    ani_boolean isUndefined;
    ani_ref filterRef;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(obj, "filter", &filterRef)) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    env->Reference_IsUndefined(filterRef, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    FsFilter filter;
    auto filterObj = static_cast<ani_object>(filterRef);
    auto [succFileSizeOver, fileSizeOver] = AniHelper::ParseInt64Option(env, filterObj, "fileSizeOver");
    if (!succFileSizeOver) {
        HILOGE("Illegal option.fileSizeOver parameter");
        return { false, nullopt };
    }
    filter.fileSizeOver = move(fileSizeOver);

    auto [succLastModifiedAfter, lastModifiedAfter] = AniHelper::ParseDoubleOption(env, filterObj, "lastModifiedAfter");
    if (!succLastModifiedAfter) {
        HILOGE("Illegal option.lastModifiedAfter parameter");
        return { false, nullopt };
    }
    filter.lastModifiedAfter = move(lastModifiedAfter);

    auto [succSuffix, suffix] = AniHelper::ParseArrayStringOption(env, filterObj, "suffix");
    if (!succSuffix) {
        HILOGE("Illegal option.suffix parameter");
        return { false, nullopt };
    }
    filter.suffix = move(suffix);

    auto [succDisplayName, displayName] = AniHelper::ParseArrayStringOption(env, filterObj, "displayName");
    if (!succDisplayName) {
        HILOGE("Illegal option.displayName parameter");
        return { false, nullopt };
    }
    filter.displayName = move(displayName);

    return { true, filter };
}

static tuple<bool, optional<FsListFileOptions>> ParseArgs(ani_env *env, ani_object obj)
{
    FsListFileOptions result;
    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succRecursion, recursion] = AniHelper::ParseBooleanOption(env, obj, "recursion");
    if (!succRecursion) {
        HILOGE("Invalid recursion");
        return { false, nullopt };
    }
    result.recursion = recursion;

    auto [succListNum, listNumRes] = AniHelper::ParseInt64Option(env, obj, "listNum");
    if (!succListNum) {
        HILOGE("Invalid listNum");
        return { false, nullopt };
    }
    result.listNum = listNumRes;

    auto [succFilter, filter] = ParseFilter(env, obj);
    if (!succFilter) {
        HILOGE("Invalid filter");
        return { false, nullopt };
    }
    result.filter = move(filter);

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
    auto [succ, result] = TypeConverter::ToAniStringList(env, fileList);
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
