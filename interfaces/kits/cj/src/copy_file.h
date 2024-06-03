/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILE_FS_COPY_FILE_H
#define OHOS_FILE_FS_COPY_FILE_H

#include <string>

#include "file_utils.h"
#include "fd_guard.h"
#include "utils.h"
#include "uv.h"

namespace OHOS {
namespace CJSystemapi {
class CopyFileImpl {
public:
    static int CopyFile(const std::string& src, const std::string& dest, int mode);
    static int CopyFile(const std::string& src, int32_t dest, int mode);
    static int CopyFile(int32_t src, const std::string& dest, int mode);
    static int CopyFile(int32_t src, int32_t dest, int mode);
};
constexpr size_t MAX_SIZE = 0x7ffff000;
}
}


#endif // OHOS_FILE_FS_COPY_FILE_H