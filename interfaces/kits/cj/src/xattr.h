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

#ifndef OHOS_FILE_FS_XATTR_H
#define OHOS_FILE_FS_XATTR_H

#include <cstdint>
#include <string>

#include "cj_common_ffi.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {
class Xattr final {
public:
    static int32_t SetSync(const char *path, const char *key, const char *value);
    static std::tuple<int32_t, char*> GetSync(const char *path, const char *key);
};
} // namespace FileFs
} // namespace CJSystemapi
} // namespace OHOS

#endif // OHOS_FILE_FS_XATTR_H
