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

#include "read_ani.h"
#include "filemgmt_libhilog.h"
#include "read_core.h"
#include "type_converter.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

ani_long ReadAni::ReadSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_int fd, ani_arraybuffer buffer)
{
    auto [succBuf, arrayBuffer] = TypeConverter::ToArrayBuffer(env, buffer);
    if (!succBuf) {
        HILOGE("Failed to parse arraybuffer");
        return -1;
    }
    auto ret = ReadCore::DoRead(fd, arrayBuffer);
    if (!ret.IsSuccess()) {
        HILOGE("Read file content failed!");
        return -1;
    }
    return ret.GetData().value();
}
} // namespace OHOS::FileManagement::ModuleFileIO::ANI