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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_NAPI_FILE_FILTER_NAPI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_NAPI_FILE_FILTER_NAPI_H

#include <mutex>
#include <thread>

#include "filemgmt_libn.h"
#include "i_file_filter.h"

namespace OHOS::FileManagement::ModuleFileIO {

class FileFilterNapi final : public IFileFilter {
public:
    FileFilterNapi(napi_env env, napi_value filterObj, LibN::NVal callback);
    bool Filter(const std::string &name) override;
    ~FileFilterNapi();

private:
    napi_env env_;
    napi_value filterObj_;
    LibN::NRef nRef_;
    bool filterFailed_ = false;

    static std::thread::id mainThreadId;
    static std::once_flag initFlag;

    static void InitMainThreadId();

    bool CallFilterFunction(const std::string &name);
    bool AsyncCallFilterFunction(const std::string &name);
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEFILTER_NAPI_FILE_FILTER_NAPI_H
