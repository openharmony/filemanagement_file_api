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

#include "hash_ani.h"

#include "filemgmt_libhilog.h"
#include "hash_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

ani_string HashAni::HashSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_string algorithm)
{
    auto [succPath, srcPath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        return nullptr;
    }

    auto [succAlg, algType] = TypeConverter::ToUTF8String(env, algorithm);
    if (!succAlg) {
        HILOGE("Invalid algorithm");
        return nullptr;
    }

    auto ret = HashCore::DoHash(srcPath, algType);
    if (!ret.IsSuccess()) {
        HILOGE("DoHash failed");
        return nullptr;
    }

    const auto &res = ret.GetData().value();
    auto [succ, result] = TypeConverter::ToAniString(env, res);
    if (!succ) {
        HILOGE("Convert hash value to ani_string failed");
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS