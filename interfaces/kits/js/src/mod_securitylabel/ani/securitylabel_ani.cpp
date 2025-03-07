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

#include "filemgmt_libhilog.h"
#include "securitylabel_ani.h"
#include "securitylabel_core.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIo {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI;
using namespace OHOS::FileManagement::ModuleSecurityLabel;

ani_int SecurityLabelAni::SetSecurityLabelSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_string level)
{
    auto [succPath, srcPath] = TypeConverter::ToUTF8StringPath(env, path);
    if (!succPath) {
        HILOGE("Invalid path");
        return EINVAL;
    }

    auto [succLevel, dataLevel] = TypeConverter::ToUTF8StringPath(env, level);
    if (!succLevel) {
        HILOGE("Invalid dataLevel");
        return EINVAL;
    }

    auto ret = DoSetSecurityLabel(srcPath, dataLevel);
    if (!ret.IsSuccess()) {
        HILOGE("file %s set securitylabel failed", srcPath.c_str());
        return -1;
    }

    return 0;
}

} // ANI
} // namespcae ModuleFileIo
} // namespcae FileManagement
} // namespcae OHOS