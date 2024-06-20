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

#ifndef OHOS_FILE_FS_MOVE_FILE_H
#define OHOS_FILE_FS_MOVE_FILE_H

#include <string>

#include "filemgmt_libn.h"
#include "file_utils.h"

namespace OHOS {
namespace CJSystemapi {


class MoveFileImpl {
public:
    static int MoveFile(const std::string& src, const std::string& dest, int mode);
};
constexpr int MODE_FORCE_MOVE = 0;
constexpr int MODE_THROW_ERR = 1;
}
}


#endif // OHOS_FILE_FS_MOVE_FILE_H