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

#include "securitylabel_ani.h"

#include "filemgmt_libhilog.h"
#include "securitylabel_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleSecurityLabel;

ani_int SecurityLabelAni::SetSecurityLabelSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_string level)
{
    auto [succPath, srcPath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        return -1;
    }

    auto [succLevel, dataLevel] = TypeConverter::ToUTF8String(env, level);
    if (!succLevel) {
        HILOGE("Invalid dataLevel");
        return -1;
    }

    auto ret = DoSetSecurityLabel(srcPath, dataLevel);
    if (!ret.IsSuccess()) {
        HILOGE("Set securitylabel failed");
        return -1;
    }

    return 0;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS