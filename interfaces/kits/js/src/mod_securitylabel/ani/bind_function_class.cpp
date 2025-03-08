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

#include <ani.h>
#include "bind_function.h"
#include "securitylabel_ani.h"

using namespace OHOS::FileManagement::ModuleFileIO::ANI;

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    *result = ANI_VERSION_1;

    static const char *className = "Lfile_securitylabel_class/securitylabel;";
    std::vector<ani_native_function> functions = {
        ani_native_function { "setSecurityLabelSync", "Lstd/core/String;Lstd/core/String;:I",
            reinterpret_cast<void *>(SecurityLabelAni::SetSecurityLabelSync) },
    };

    return BindClass(vm, className, functions);
}
