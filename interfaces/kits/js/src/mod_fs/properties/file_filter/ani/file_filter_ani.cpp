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

#include "file_filter_ani.h"

#include "ani_signature.h"
#include "filemgmt_libhilog.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO::ANI::AniSignature;

inline static const int32_t ANI_SCOPE_SIZE = 16;

bool FileFilterAni::Filter(const std::string &name)
{
    if (filterFailed_) {
        return false;
    }

    ani_size scopeSize = ANI_SCOPE_SIZE;
    ani_status status = env_->CreateLocalScope(scopeSize);
    if (status != ANI_OK) {
        HILOGE("Failed to create local scope, status: %{public}d", status);
        filterFailed_ = true;
        return false;
    }

    auto [succ, nameValue] = TypeConverter::ToAniString(env_, name);
    if (!succ) {
        HILOGE("Failed to create ANI string");
        filterFailed_ = true;
        status = env_->DestroyLocalScope();
        if (status != ANI_OK) {
            HILOGE("Failed to destroy local scope, status: %{public}d", status);
        }
        return false;
    }

    ani_boolean result;
    status = env_->Object_CallMethod_Boolean(filterObj_, filterMethod_, &result, nameValue);
    if (status != ANI_OK) {
        HILOGE("Failed to call filter function, status: %{public}d", status);
        filterFailed_ = true;
        status = env_->DestroyLocalScope();
        if (status != ANI_OK) {
            HILOGE("Failed to destroy local scope, status: %{public}d", status);
        }
        return false;
    }

    status = env_->DestroyLocalScope();
    if (status != ANI_OK) {
        HILOGE("Failed to destroy local scope, status: %{public}d", status);
        filterFailed_ = true;
        return false;
    }

    return static_cast<bool>(result);
}

FileFilterAni::~FileFilterAni() {}

bool FileFilterAni::HasException() const
{
    ani_boolean hasError = false;
    env_->ExistUnhandledError(&hasError);
    return static_cast<bool>(hasError);
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
