/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_EXT_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_EXT_CORE_H

#include <dirent.h>
#include <optional>

#include "filemgmt_libfs.h"
#include "i_file_filter.h"

namespace OHOS::FileManagement::ModuleFileIO {

namespace {
constexpr int32_t FILTER_MATCH = 1;
constexpr int32_t FILTER_DISMATCH = 0;

struct NameListArg {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

struct OptionArgs {
    std::shared_ptr<IFileFilter> fileFilter = nullptr;
    int listNum = 0;
    int countNum = 0;
    bool recursion = false;
    std::string path = "";
    std::string originalPath = "";
    void Clear()
    {
        fileFilter = nullptr;
        listNum = 0;
        countNum = 0;
        recursion = false;
        path = "";
        originalPath = "";
    }
};

} // namespace

struct ListFileExtOptions {
    bool recursion = false;
    std::optional<int64_t> listNum = 0;
    std::shared_ptr<IFileFilter> fileFilter = nullptr;
};

class ListFileExtCore {
public:
    static FsResult<std::vector<std::string>> DoListFileExt(
        const std::string &path, const std::optional<ListFileExtOptions> &options = std::nullopt);
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_EXT_CORE_H
