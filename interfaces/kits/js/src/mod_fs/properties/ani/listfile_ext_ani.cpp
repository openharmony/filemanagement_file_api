/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "listfile_ext_ani.h"

#include "ani_signature.h"
#include "error_handler.h"
#include "file_filter_ani.h"
#include "filemgmt_libhilog.h"
#include "listfile_ext_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

static std::tuple<bool, shared_ptr<IFileFilter>> ParseFileFilter(ani_env *env, ani_object obj)
{
    ani_ref filterRef;
    ani_status status = env->Object_GetPropertyByName_Ref(obj, "fileFilter", &filterRef);
    if (status != ANI_OK) {
        HILOGE("Failed to get fileFilter property, ret: %{public}d", status);
        return { false, nullptr };
    }

    ani_boolean isUndefined;
    env->Reference_IsUndefined(filterRef, &isUndefined);
    if (isUndefined) {
        return { true, nullptr };
    }

    AniCache &aniCache = AniCache::GetInstance();
    auto classDesc = FS::FileFilter::classDesc;
    auto filterSig = FS::FileFilter::filterSig;
    auto [ret, cls] = aniCache.GetClass(env, classDesc);
    if (ret != ANI_OK) {
        HILOGE("Failed to find class: %{public}s. ret: %{public}d", classDesc.c_str(), ret);
        return { false, nullptr };
    }

    ani_method filterMethod;
    tie(ret, filterMethod) = aniCache.GetMethod(env, classDesc, "filter", filterSig);
    if (ret != ANI_OK) {
        HILOGE("Failed to find filter method: %{public}s. ret: %{public}d", filterSig.c_str(), ret);
        return { false, nullptr };
    }

    auto fileFilter = CreateSharedPtr<FileFilterAni>(env, static_cast<ani_object>(filterRef), filterMethod);
    if (!fileFilter) {
        HILOGE("Failed to request heap memory.");
        return { false, nullptr };
    }

    return { true, fileFilter };
}

static std::tuple<bool, std::optional<ListFileExtOptions>> ParseArgs(ani_env *env, ani_object obj)
{
    ListFileExtOptions result;
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
    auto [succFileFilter, fileFilter] = ParseFileFilter(env, obj);
    if (!succFileFilter) {
        HILOGE("Invalid fileFilter");
        return { false, nullopt };
    }
    result.fileFilter = fileFilter;

    return { true, result };
}

ani_array ListFileExtAni::ListFileExtSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object obj)
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

    auto ret = ListFileExtCore::DoListFileExt(srcPath, opt);
    if (opt.has_value() && opt.value().fileFilter) {
        auto aniFilter = std::static_pointer_cast<FileFilterAni>(opt.value().fileFilter);
        if (aniFilter && aniFilter->HasException()) {
            HILOGE("Filter callback threw exception");
            // Note: No need to throw exception manually
            // ANI runtime will propagate the pending exception to ETS layer
            return nullptr;
        }
    }
    if (!ret.IsSuccess()) {
        HILOGE("DoListFileExt failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    auto fileList = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniStringList(env, fileList);
    if (!succ) {
        HILOGE("Convert listfile ext result to ani string array failed");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
