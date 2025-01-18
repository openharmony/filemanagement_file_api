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

#ifndef OHOS_STATVFS_IMPL_H
#define OHOS_STATVFS_IMPL_H

#include <cstdint>
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace Statvfs {
class StatvfsImpl {
public:
    static std::tuple<int32_t, int64_t> GetFreeSize(const char* path);
    static std::tuple<int32_t, int64_t> GetTotalSize(const char* path);
};
}
}
}

#endif // OHOS_STATVFS_IMPL_H