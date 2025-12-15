/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ut_fs_utils.h"

#include <gtest/gtest.h>
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {

std::tuple<bool, FileInfo> GenerateFileInfoFromPath(const std::string &path)
{
    size_t length = path.length() + 1;
    auto chars = std::make_unique<char[]>(length);
    auto ret = strncpy_s(chars.get(), length, path.c_str(), path.length());
    if (ret != EOK) {
        GTEST_LOG_(ERROR) << "Copy file path failed! ret: " << ret;
        return { false, FileInfo { false, {}, {} } };
    }
    return { true, FileInfo { true, std::move(chars), {} } };
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
