/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_ANI_FILE_FILTER_ANI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_ANI_FILE_FILTER_ANI_H

#include <ani.h>

#include "i_file_filter.h"

namespace OHOS::FileManagement::ModuleFileIO {
namespace ANI {

class FileFilterAni final : public IFileFilter {
public:
    FileFilterAni(ani_env *env, ani_object filterObj, ani_method filterMethod)
        : env_(env), filterObj_(filterObj), filterMethod_(filterMethod) {};
    bool Filter(const std::string &name) override;
    ~FileFilterAni();
    bool HasException() const;

private:
    ani_env *env_;
    ani_object filterObj_;
    ani_method filterMethod_;
    bool filterFailed_ = false;
};
} // namespace ANI
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_ANI_FILE_FILTER_ANI_H
